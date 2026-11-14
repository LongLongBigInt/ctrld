/*
 * ctrld - TCP remote control
 *
 * Protocol: [u16 big-endian length][command bytes]
 */

#if !defined(CTRLD_FREESTANDING)
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include "freestanding.h"
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

#define LISTEN_BACKLOG 8
#define CTRLD_DEFAULT_PORT 2323
#define CTRLD_DEFAULT_SHELL "/bin/sh"
#define CTRLD_USAGE \
	"usage: ctrld [-p PORT] [-s SHELL] [-h]\n" \
	"  -p PORT   listen port (default " STR(CTRLD_DEFAULT_PORT) ")\n" \
	"  -s SHELL  shell path (default " CTRLD_DEFAULT_SHELL ")\n" \
	"  -h        show this help"

enum exit_code {
	EXIT_OK = 0,
	EXIT_SOCKET,
	EXIT_USAGE,
	EXIT_BIND,
	EXIT_LISTEN,
	EXIT_ACCEPT,
	EXIT_SHELL,
	EXIT_EXEC = 127,
};

enum parse_status {
	PARSE_OK,
	PARSE_ERROR,
	PARSE_HELP,
};

static bool has_nul(const char *p, size_t n)
{
	for (size_t i = 0; i < n; i++)
		if (!p[i])
			return true;
	return false;
}

static void write_all(int fd, const void *buf, size_t n)
{
	const char *p = buf;
	while (n) {
		ssize_t r = write(fd, p, n);

		if (r < 0) {
			if (errno == EINTR)
				continue;
			return;
		}
		if (!r)
			return;
		p += r;
		n -= (size_t)r;
	}
}

#define fprintln(file, msg) write_all(file, msg "\n", sizeof(msg))
#define eprintln(msg) fprintln(STDERR_FILENO, msg)

static bool recv_exact(int fd, void *buf, size_t n)
{
	char *p = buf;
	while (n) {
		ssize_t r = read(fd, p, n);

		if (r < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}
		if (!r)
			return false;
		p += r;
		n -= (size_t)r;
	}
	return true;
}

static struct config {
	uint16_t port;
	const char *shell;
} cfg = {
	.port = CTRLD_DEFAULT_PORT,
	.shell = CTRLD_DEFAULT_SHELL
};

static bool parse_port(const char *s, uint16_t *out)
{
	unsigned long v = 0;
	
	if (!*s)
		return false;
	for (; *s; s++) {
		if (*s < '0' || *s > '9')
			return false;
		v = v * 10 + (unsigned)(*s - '0');
		if (v > 65535)
			return false;
	}
	if (!v)
		return false;
	*out = (uint16_t)v;
	return true;
}

static const char *arg_value(char *argv[], int argc, int *i, const char *a)
{
	if (a[2])
		return a + 2;
	if (++*i < argc)
		return argv[*i];
	return NULL;
}

static enum parse_status parse_args(char *argv[], int argc)
{
	for (int i = 0; i < argc; i++) {
		const char *a = argv[i], *v;

		if (a[0] != '-' || !a[1])
			return PARSE_ERROR;

		switch (a[1]) {
		case 'h':
			if (a[2])
				return PARSE_ERROR;
			return PARSE_HELP;
		case 'p':
			v = arg_value(argv, argc, &i, a);
			if (!v || !parse_port(v, &cfg.port))
				return PARSE_ERROR;
			break;
		case 's':
			v = arg_value(argv, argc, &i, a);
			if (!v || !*v)
				return PARSE_ERROR;
			cfg.shell = v;
			break;
		default:
			return PARSE_ERROR;
		}
	}

	return PARSE_OK;
}

static _Noreturn void handle(int sock)
{
	uint16_t hdr;
	static char cmd[1 << 16];

	if (!recv_exact(sock, &hdr, sizeof hdr))
		exit(EXIT_OK);

	size_t len = ntohs(hdr);
	if (!len || !recv_exact(sock, cmd, len))
		exit(EXIT_OK);
	
	if (has_nul(cmd, len))
		exit(EXIT_OK);

	cmd[len] = 0;

	int fd = open("/dev/null", O_RDONLY, 0);
	if (fd >= 0) {
		dup2(fd, STDIN_FILENO);
		if (fd > 0)
			close(fd);
	}

	dup2(sock, STDOUT_FILENO);
	dup2(sock, STDERR_FILENO);
	if (sock > 2)
		close(sock);

	char *argv[] = { (char *)cfg.shell, "-c", cmd, NULL };
	execve(cfg.shell, argv, environ);

	exit(EXIT_EXEC);
}

static _Noreturn void run_server(int sock)
{
	while (true) {
		int c = accept(sock, NULL, NULL);
		if (c < 0) {
			if (errno == EINTR)
				continue;
			exit(EXIT_ACCEPT);
		}

		pid_t pid = fork();
		if (!pid) {
			close(sock);
			/* Do not leak SIG_IGN into the shell. */
			signal(SIGCHLD, SIG_DFL);
			handle(c);
		}
		close(c);
	}
}

static int create_server(int *fd)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return EXIT_SOCKET;

	int one = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, (socklen_t)sizeof one);

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(cfg.port),
	};

	if (bind(s, (const struct sockaddr *)&addr, (socklen_t)sizeof addr) < 0) {
		close(s);
		return EXIT_BIND;
	}
	
	if (listen(s, LISTEN_BACKLOG) < 0) {
		close(s);
		return EXIT_LISTEN;
	}

	*fd = s;
	return EXIT_OK;
}

int main(int argc, char *argv[])
{
	signal(SIGCHLD, SIG_IGN);

	switch (parse_args(argv + 1, argc - 1)) {
	case PARSE_HELP:
		fprintln(STDOUT_FILENO, CTRLD_USAGE);
		return EXIT_OK;
	case PARSE_ERROR:
		eprintln(CTRLD_USAGE);
		return EXIT_USAGE;
	case PARSE_OK:
		break;
	}

	if (access(cfg.shell, X_OK) < 0) {
		eprintln("ctrld: shell not executable");
		return EXIT_SHELL;
	}

	int server_fd, ec;
	if ((ec = create_server(&server_fd)) != EXIT_OK) {
		eprintln("ctrld: can not start server");
		return ec;
	}

	run_server(server_fd);
}
