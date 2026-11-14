#ifndef CTRLD_ARCH_S390X_H
#define CTRLD_ARCH_S390X_H

#define NR_fork 2
#define NR_read 3
#define NR_write 4
#define NR_close 6
#define NR_execve 11
#define NR_rt_sigaction 174
#define NR_exit_group 248
#define NR_openat 288
#define NR_faccessat 300
#define NR_dup3 326
#define NR_socket 359
#define NR_bind 361
#define NR_listen 363
#define NR_accept4 364
#define NR_setsockopt 366

static inline long sc(long n, long a, long b, long c, long d, long e)
{
	register long r1 __asm__("1") = n;
	register long r2 __asm__("2") = a;
	register long r3 __asm__("3") = b;
	register long r4 __asm__("4") = c;
	register long r5 __asm__("5") = d;
	register long r6 __asm__("6") = e;

	__asm__ volatile("svc 0"
		: "+r"(r2)
		: "r"(r1), "r"(r3), "r"(r4), "r"(r5), "r"(r6)
		: "memory", "cc");
	return r2;
}

__asm__(
	".global _start\n"
	"_start:\n"
	"lg %r2, 0(%r15)\n"
	"la %r3, 8(%r15)\n"
	"brasl %r14, cstart\n"
);

#endif
