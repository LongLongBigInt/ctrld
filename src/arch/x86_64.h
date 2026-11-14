#ifndef CTRLD_ARCH_X86_64_H
#define CTRLD_ARCH_X86_64_H

#define NR_read 0
#define NR_write 1
#define NR_close 3
#define NR_rt_sigaction 13
#define NR_socket 41
#define NR_bind 49
#define NR_listen 50
#define NR_setsockopt 54
#define NR_fork 57
#define NR_execve 59
#define NR_exit_group 231
#define NR_openat 257
#define NR_faccessat 269
#define NR_accept4 288
#define NR_dup3 292

static inline long sc(long n, long a, long b, long c, long d, long e)
{
	long ret;
	register long r10 __asm__("r10") = d;
	register long r8 __asm__("r8") = e;

	__asm__ volatile("syscall"
		: "=a"(ret)
		: "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8)
		: "rcx", "r11", "cc", "memory");
	return ret;
}

__asm__(
	".global _start\n"
	"_start:\n"
	"mov (%rsp), %rdi\n"
	"lea 8(%rsp), %rsi\n"
	"and $-16, %rsp\n"
	"call cstart\n"
);

#endif
