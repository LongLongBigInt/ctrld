# ctrld - TCP remote control
#
#   make                              freestanding, host os/arch
#   make OS=linux ARCH=aarch64 CC=aarch64-linux-gnu-gcc
#   make FREESTANDING=0               libc build (gdb/strace)
#
# Artifact: build/ctrld-<os>-<arch>[-freestanding]

OS   ?= linux
ARCH ?= $(shell uname -m)
CC   ?= cc

FREESTANDING ?= 1
BIN := build/ctrld-$(OS)-$(ARCH)$(if $(filter 1,$(FREESTANDING)),-freestanding)

CFLAGS  := -std=c11 -Os -Wall -Wextra
LDFLAGS :=
ifeq ($(FREESTANDING),1)
CFLAGS  += -DCTRLD_FREESTANDING -ffreestanding -fno-builtin -fno-common \
           -fno-ident -fno-stack-protector -fno-pie -fno-pic \
           -fno-asynchronous-unwind-tables -fno-unwind-tables \
           -ffunction-sections -fdata-sections
LDFLAGS := -nostdlib -static -no-pie -Wl,--build-id=none -Wl,--gc-sections \
           -Wl,-z,noseparate-code -s
endif

HDRS := $(wildcard src/*.h src/arch/*.h)

all: $(BIN)

$(BIN): src/ctrld.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -rf build

.PHONY: all clean
