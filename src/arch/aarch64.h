#ifndef CTRLD_ARCH_AARCH64_H
#define CTRLD_ARCH_AARCH64_H

#define NR_dup3 24
#define NR_faccessat 48
#define NR_openat 56
#define NR_close 57
#define NR_read 63
#define NR_write 64
#define NR_exit_group 94
#define NR_rt_sigaction 134
#define NR_socket 198
#define NR_bind 200
#define NR_listen 201
#define NR_setsockopt 208
#define NR_clone 220
#define NR_execve 221
#define NR_accept4 242

static inline long sc(long n, long a, long b, long c, long d, long e)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0") = a;
	register long x1 __asm__("x1") = b;
	register long x2 __asm__("x2") = c;
	register long x3 __asm__("x3") = d;
	register long x4 __asm__("x4") = e;

	__asm__ volatile("svc 0"
		: "+r"(x0)
		: "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4)
		: "memory", "cc");
	return x0;
}

__asm__(
	".global _start\n"
	"_start:\n"
	"ldr x0, [sp]\n"
	"add x1, sp, #8\n"
	"bl cstart\n"
);

#endif
