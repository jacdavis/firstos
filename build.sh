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
${CROSS_COMPILER}-g++ -c pmm.c++ -o ${BIN_DIR}/pmm.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c paging.c++ -o ${BIN_DIR}/paging.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
${CROSS_COMPILER}-g++ -c process.c++ -o ${BIN_DIR}/process.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

echo "Building user test program..."
${CROSS_COMPILER}-gcc -ffreestanding -O2 -c usertest.c -o ${BIN_DIR}/usertest.o
${CROSS_COMPILER}-ld -T user_linker.ld -o ${BIN_DIR}/usertest.elf ${BIN_DIR}/usertest.o
${CROSS_COMPILER}-objcopy -I binary -O elf32-i386 -B i386 ${BIN_DIR}/usertest.elf ${BIN_DIR}/usertest_embedded.o

echo "Assembling assembly sources..."
${CROSS_COMPILER}-as boot.s -o ${BIN_DIR}/boot.o
${CROSS_COMPILER}-as gdt.s -o ${BIN_DIR}/gdt_asm.o
${CROSS_COMPILER}-as interrupts.s -o ${BIN_DIR}/interrupts.o
${CROSS_COMPILER}-as io.s -o ${BIN_DIR}/io.o
${CROSS_COMPILER}-as terminal_cursor.s -o ${BIN_DIR}/terminal_cursor.o
${CROSS_COMPILER}-as paging.s -o ${BIN_DIR}/paging_asm.o
${CROSS_COMPILER}-as process.s -o ${BIN_DIR}/process_asm.o

echo "Linking kernel..."
${CROSS_COMPILER}-g++ -T linker.ld -o ${BIN_DIR}/myos.bin -ffreestanding -O2 -nostdlib \
    ${BIN_DIR}/boot.o ${BIN_DIR}/kernel.o ${BIN_DIR}/gdt.o ${BIN_DIR}/gdt_asm.o ${BIN_DIR}/idt.o ${BIN_DIR}/interrupts.o ${BIN_DIR}/keyboard.o ${BIN_DIR}/io.o ${BIN_DIR}/terminal_cursor.o \
    ${BIN_DIR}/pmm.o ${BIN_DIR}/paging.o ${BIN_DIR}/paging_asm.o ${BIN_DIR}/process.o ${BIN_DIR}/process_asm.o \
    ${BIN_DIR}/usertest_embedded.o \
    -lgcc

echo "Build successful! Kernel: ${BIN_DIR}/myos.bin"
echo ""
echo "To run with QEMU:"
echo "  qemu-system-i386 -kernel ${BIN_DIR}/myos.bin"
