#ifndef CTRLD_ARCH_RISCV64_H
#define CTRLD_ARCH_RISCV64_H

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
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0") = a;
	register long a1 __asm__("a1") = b;
	register long a2 __asm__("a2") = c;
	register long a3 __asm__("a3") = d;
	register long a4 __asm__("a4") = e;

	__asm__ volatile("ecall"
		: "+r"(a0)
		: "r"(a7), "r"(a1), "r"(a2), "r"(a3), "r"(a4)
		: "memory");
	return a0;
}

__asm__(
	".global _start\n"
	"_start:\n"
	"ld a0, 0(sp)\n"
	"addi a1, sp, 8\n"
	"andi sp, sp, -16\n"
	"call cstart\n"
);

#endif
