#!/bin/bash
rm out/*
nasm -f bin boot/boot.asm -o out/boot.bin
nasm -f bin boot/loader.asm -o out/loader.bin
nasm -f elf32 boot/kernel.asm -o out/kernel0.o

# TODO: 1. Make this work with neko-gcc
#       2. Use a build system ;D

export GCC_FLAGS="-std=gnu99 -ffreestanding -mno-red-zone -fno-exceptions -nostdlib -Wall -Wextra -Werror -fno-pie -O2"
export INCLUDE_DIRS="-I kernel/include -I libc/include"
export KERNEL_FILES="kernel/kernel.c kernel/tty.c out/kernel0.o"
export LIBC_FILES="libc/string/memcmp.c libc/string/memcpy.c libc/string/memset.c libc/string/strlen.c"

gcc -m32 $KERNEL_FILES $LIBC_FILES -o out/NEKOKRNL -T kernel.ld $GCC_FLAGS $INCLUDE_DIRS
