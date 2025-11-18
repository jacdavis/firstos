#!/bin/bash

set -e

CROSS_COMPILER=~/opt/cross/bin/i686-elf

echo "Building FirstOS..."

echo "Compiling C++ sources..."
${CROSS_COMPILER}-g++ -c kernel.c++ -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c gdt.c++ -o gdt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c idt.c++ -o idt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c keyboard.c++ -o keyboard.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

echo "Assembling assembly sources..."
${CROSS_COMPILER}-as boot.s -o boot.o
${CROSS_COMPILER}-as gdt.s -o gdt_asm.o
${CROSS_COMPILER}-as interrupts.s -o interrupts.o
${CROSS_COMPILER}-as io.s -o io.o
${CROSS_COMPILER}-as terminal_cursor.s -o terminal_cursor.o

echo "Linking kernel..."
${CROSS_COMPILER}-g++ -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib \
    boot.o kernel.o gdt.o gdt_asm.o idt.o interrupts.o keyboard.o io.o terminal_cursor.o \
    -lgcc

echo "Build successful! Kernel: myos.bin"
echo ""
echo "To run with QEMU:"
echo "  qemu-system-i386 -kernel myos.bin"
