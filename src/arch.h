#ifndef CTRLD_ARCH_H
#define CTRLD_ARCH_H

#if defined(__x86_64__)
#include "arch/x86_64.h"
#elif defined(__i386__)
#include "arch/i386.h"
#elif defined(__aarch64__)
#include "arch/aarch64.h"
#elif defined(__riscv) && __riscv_xlen == 64
#include "arch/riscv64.h"
#elif defined(__arm__)
#include "arch/armv7.h"
#elif defined(__s390x__)
#include "arch/s390x.h"
#else
#error "unsupported architecture"
#endif

#endif
