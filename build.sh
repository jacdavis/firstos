#!/bin/bash

set -e

CROSS_COMPILER=~/opt/cross/bin/i686-elf
BIN_DIR=bin

echo "Building FirstOS..."

# Create bin directory if it doesn't exist
mkdir -p ${BIN_DIR}

echo "Compiling C++ sources..."
${CROSS_COMPILER}-g++ -c kernel.c++ -o ${BIN_DIR}/kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c gdt.c++ -o ${BIN_DIR}/gdt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c idt.c++ -o ${BIN_DIR}/idt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c keyboard.c++ -o ${BIN_DIR}/keyboard.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

echo "Assembling assembly sources..."
${CROSS_COMPILER}-as boot.s -o ${BIN_DIR}/boot.o
${CROSS_COMPILER}-as gdt.s -o ${BIN_DIR}/gdt_asm.o
${CROSS_COMPILER}-as interrupts.s -o ${BIN_DIR}/interrupts.o
${CROSS_COMPILER}-as io.s -o ${BIN_DIR}/io.o
${CROSS_COMPILER}-as terminal_cursor.s -o ${BIN_DIR}/terminal_cursor.o

echo "Linking kernel..."
${CROSS_COMPILER}-g++ -T linker.ld -o ${BIN_DIR}/myos.bin -ffreestanding -O2 -nostdlib \
    ${BIN_DIR}/boot.o ${BIN_DIR}/kernel.o ${BIN_DIR}/gdt.o ${BIN_DIR}/gdt_asm.o ${BIN_DIR}/idt.o ${BIN_DIR}/interrupts.o ${BIN_DIR}/keyboard.o ${BIN_DIR}/io.o ${BIN_DIR}/terminal_cursor.o \
    -lgcc

echo "Build successful! Kernel: ${BIN_DIR}/myos.bin"
echo ""
echo "To run with QEMU:"
echo "  qemu-system-i386 -kernel ${BIN_DIR}/myos.bin"
