/* ctrld freestanding - linux */

#ifndef CTRLD_FREESTANDING_H
#define CTRLD_FREESTANDING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef long ssize_t;
typedef int pid_t;
typedef unsigned int socklen_t;
typedef unsigned int mode_t;
typedef unsigned short sa_family_t;
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
typedef void (*sighandler_t)(int);

struct in_addr {
	in_addr_t s_addr;
};

struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};

struct sockaddr_in {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
	unsigned char sin_zero[8];
};

#define	STDIN_FILENO 0
#define	STDOUT_FILENO 1
#define	STDERR_FILENO 2

#define EINTR 4
#define AF_INET 2
#define SOCK_STREAM 1
#define SOL_SOCKET 1
#define SO_REUSEADDR 2
#define SIGCHLD 17
#define SIG_IGN ((sighandler_t)1)
#define SIG_DFL ((sighandler_t)0)
#define SIG_ERR ((sighandler_t)-1)
#define AT_FDCWD (-100)
#define X_OK 1
#define O_RDONLY 0

static int errno;
static char **environ;

/* Kernel return value -> value, or -1 with errno set, as libc does. */
static long sysret(long r)
{
	if (r >= 0)
		return r;
	errno = (int)-r;
	return -1;
}

#include "arch.h"

static inline uint16_t ntohs(uint16_t v)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return v;
#else
	return __builtin_bswap16(v);
#endif
}

static inline uint16_t htons(uint16_t v)
{
	return ntohs(v);
}

static inline pid_t fork(void)
{
#ifdef NR_clone
	return (pid_t)sysret(sc(NR_clone, SIGCHLD, 0, 0, 0, 0));
#else
	return (pid_t)sysret(sc(NR_fork, 0, 0, 0, 0, 0));
#endif
}

static inline ssize_t read(int fd, void *buf, size_t n)
{
	return (ssize_t)sysret(sc(NR_read, fd, (long)buf, n, 0, 0));
}

static inline ssize_t write(int fd, const void *buf, size_t n)
{
	return (ssize_t)sysret(sc(NR_write, fd, (long)buf, n, 0, 0));
}

static inline int close(int fd)
{
	return (int)sysret(sc(NR_close, fd, 0, 0, 0, 0));
}

/* The targets have openat()/faccessat() only; no legacy open()/access(). */
static inline int open(const char *path, int flags, mode_t mode)
{
	return (int)sysret(sc(NR_openat, AT_FDCWD, (long)path, flags, mode, 0));
}

static inline int access(const char *path, int mode)
{
	return (int)sysret(sc(NR_faccessat, AT_FDCWD, (long)path, mode, 0, 0));
}

/* Same contract as dup2(), but the kernel only grew dup3(): equal fds are a
 * no-op, as in dup2(), and dup3() would report EINVAL for them. */
static inline int dup2(int oldfd, int newfd)
{
	if (oldfd == newfd)
		return newfd;
	return (int)sysret(sc(NR_dup3, oldfd, newfd, 0, 0, 0));
}

static inline int socket(int domain, int type, int protocol)
{
	return (int)sysret(sc(NR_socket, domain, type, protocol, 0, 0));
}

static inline int bind(int fd, const struct sockaddr *addr, socklen_t len)
{
	return (int)sysret(sc(NR_bind, fd, (long)addr, len, 0, 0));
}

static inline int listen(int fd, int backlog)
{
	return (int)sysret(sc(NR_listen, fd, backlog, 0, 0, 0));
}

static inline int accept(int fd, struct sockaddr *addr, socklen_t *len)
{
	return (int)sysret(sc(NR_accept4, fd, (long)addr, (long)len, 0, 0));
}

static inline int setsockopt(int fd, int level, int optname, const void *val, socklen_t len)
{
	return (int)sysret(sc(NR_setsockopt, fd, level, optname, (long)val, len));
}

static inline int execve(const char *path, char *const argv[], char *const envp[])
{
	return (int)sysret(sc(NR_execve, (long)path, (long)argv, (long)envp, 0, 0));
}

static inline _Noreturn void exit(int status)
{
	sc(NR_exit_group, status, 0, 0, 0, 0);
	__builtin_unreachable();
}

/* Kernel ABI layout; sa_mask is always 8 bytes (64-bit sigset), also on 32-bit. */
struct k_sigaction {
	sighandler_t handler;
	unsigned long flags;
	sighandler_t restorer;
	uint64_t mask;
};

static inline sighandler_t signal(int sig, sighandler_t handler)
{
	struct k_sigaction sa = { .handler = handler }, old;

	if (sysret(sc(NR_rt_sigaction, sig, (long)&sa, (long)&old, 8, 0)) < 0)
		return SIG_ERR;
	return old.handler;
}

int main(int argc, char *argv[]);

/* Kernel enters with sp -> argc, argv, NULL, envp, NULL, auxv. */
_Noreturn void cstart(int argc, char **argv)
{
	environ = argv + argc + 1;
	exit(main(argc, argv));
}

#endif
