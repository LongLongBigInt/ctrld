#ifndef CTRLD_ARCH_I386_H
#define CTRLD_ARCH_I386_H

#define NR_fork 2
#define NR_read 3
#define NR_write 4
#define NR_close 6
#define NR_execve 11
#define NR_rt_sigaction 174
#define NR_exit_group 252
#define NR_openat 295
#define NR_faccessat 307
#define NR_dup3 330
#define NR_socket 359
#define NR_bind 361
#define NR_listen 363
#define NR_accept4 364
#define NR_setsockopt 366

static inline long sc(long n, long a, long b, long c, long d, long e)
{
	long ret;

	__asm__ volatile("int $0x80"
		: "=a"(ret)
		: "a"(n), "b"(a), "c"(b), "d"(c), "S"(d), "D"(e)
		: "memory", "cc");
	return ret;
}

__asm__(
	".global _start\n"
	"_start:\n"
	"mov (%esp), %eax\n"
	"lea 4(%esp), %ecx\n"
	"and $-16, %esp\n"
	"sub $8, %esp\n"
	"push %ecx\n"
	"push %eax\n"
	"call cstart\n"
);

#endif
