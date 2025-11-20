#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "terminal.h"
#include "pmm.h"
#include "paging.h"
#include "process.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
//#if defined(__linux__)
//#error "You are not using a cross-compiler, you will most certainly run into trouble"
//#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
//#if !defined(__i386__)
//#error "This tutorial needs to be compiled with a ix86-elf compiler"
//#endif

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_update_cursor();

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
	
	terminal_update_cursor();
}

extern "C" void terminal_update_cursor_asm(uint16_t pos);

void terminal_update_cursor() {
	uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
	terminal_update_cursor_asm(pos);
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) 
{
	if (terminal_row == 0) {
		terminal_row = 1;
		terminal_column = 0;
	}
	
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 1;
	} else if (c == '\b') {
		if (terminal_column > 0) {
			terminal_column--;
			terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		} else if (terminal_row > 1) {
			terminal_row--;
			terminal_column = VGA_WIDTH - 1;
			terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		}
	} else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 1;
		}
	}
	terminal_update_cursor();
}

void terminal_move_cursor_left() {
	if (terminal_column > 0) {
		terminal_column--;
		terminal_update_cursor();
	} else if (terminal_row > 1) {
		terminal_row--;
		terminal_column = VGA_WIDTH - 1;
		terminal_update_cursor();
	}
}

void terminal_move_cursor_right() {
	if (terminal_column < VGA_WIDTH - 1) {
		terminal_column++;
		terminal_update_cursor();
	} else if (terminal_row < VGA_HEIGHT - 1) {
		terminal_row++;
		terminal_column = 0;
		terminal_update_cursor();
	}
}

void terminal_move_cursor_up() {
	if (terminal_row > 1) {
		terminal_row--;
		terminal_update_cursor();
	}
}

void terminal_move_cursor_down() {
	if (terminal_row < VGA_HEIGHT - 1) {
		terminal_row++;
		terminal_update_cursor();
	}
}

void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

void terminal_write_welcome() {
	const char* msg = "Welcome to jacnix";
	size_t len = strlen(msg);
	for (size_t i = 0; i < len; i++) {
		terminal_putentryat(msg[i], terminal_color, i, 0);
	}
	terminal_row = 1;
	terminal_column = 0;
	terminal_update_cursor();
}

extern uint32_t _kernel_end;
extern uint8_t _binary_bin_usertest_elf_start[];
extern uint8_t _binary_bin_usertest_elf_end[];

void print_hex(uint32_t val) {
    char buf[11] = "0x";
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (val >> (i * 4)) & 0xF;
        buf[9-i] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
    }
    buf[10] = 0;
    terminal_writestring(buf);
}

extern "C" void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();

	/* Initialize GDT and IDT */
	gdt_init();
	idt_init();
	keyboard_init();

	terminal_write_welcome();
	terminal_writestring("\nOS Starting...\n");
	
	terminal_writestring("Initializing PMM...\n");
	pmm_init(0, 32 * 1024 * 1024);
	terminal_writestring("PMM OK\n");
	
	terminal_writestring("Enabling paging...\n");
	paging_init();
	terminal_writestring("Paging OK\n");
	
	terminal_writestring("System ready!\n");
	
	process_init();
	terminal_writestring("Process subsystem initialized\n");
	
	uint32_t elf_size = (uint32_t)_binary_bin_usertest_elf_end - (uint32_t)_binary_bin_usertest_elf_start;
	terminal_writestring("ELF size: ");
	print_hex(elf_size);
	terminal_writestring("\n");
	
	struct process* proc = process_create_from_elf(_binary_bin_usertest_elf_start, elf_size);
	
	if (proc) {
		terminal_writestring("Process loaded! EIP=");
		print_hex(proc->eip);
		terminal_writestring(" ESP=");
		print_hex(proc->esp);
		terminal_writestring("\n");
		
		// Check if code is actually there
		paging_switch_directory(proc->page_dir);
		uint8_t* code_ptr = (uint8_t*)proc->eip;
		terminal_writestring("Code at EIP: ");
		print_hex(code_ptr[0]);
		terminal_writestring(" ");
		print_hex(code_ptr[1]);
		terminal_writestring(" ");
		print_hex(code_ptr[2]);
		terminal_writestring(" ");
		print_hex(code_ptr[3]);
		terminal_writestring("\n");
		paging_switch_directory(kernel_directory);
		
		terminal_writestring("Switching to ring 3...\n");
		process_switch_to(proc);
		terminal_writestring("Back in kernel (shouldn't happen!)\n");
	} else {
		terminal_writestring("Process load FAILED\n");
	}
	
	/* Enable interrupts */
	asm volatile("sti");
	
	/* Hang forever */
	while(1) {
		asm volatile("hlt");
	}
}
