#ifndef CTRLD_ARCH_ARMV7_H
#define CTRLD_ARCH_ARMV7_H

#define NR_fork 2
#define NR_read 3
#define NR_write 4
#define NR_close 6
#define NR_execve 11
#define NR_rt_sigaction 174
#define NR_exit_group 248
#define NR_socket 281
#define NR_bind 282
#define NR_listen 284
#define NR_setsockopt 294
#define NR_openat 322
#define NR_faccessat 334
#define NR_dup3 358
#define NR_accept4 366

static inline long sc(long n, long a, long b, long c, long d, long e)
{
	register long r7 __asm__("r7") = n;
	register long r0 __asm__("r0") = a;
	register long r1 __asm__("r1") = b;
	register long r2 __asm__("r2") = c;
	register long r3 __asm__("r3") = d;
	register long r4 __asm__("r4") = e;

	__asm__ volatile("svc 0"
		: "+r"(r0)
		: "r"(r7), "r"(r1), "r"(r2), "r"(r3), "r"(r4)
		: "memory", "cc");
	return r0;
}

__asm__(
	".global _start\n"
	"_start:\n"
	"ldr r0, [sp]\n"
	"add r1, sp, #4\n"
	"bic sp, sp, #7\n"
	"bl cstart\n"
);

#endif
