#include "keyboard.h"
#include "terminal.h"
#include <stdint.h>

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

void keyboard_init() {
    // Keyboard already initialized by BIOS/bootloader
}

extern "C" uint8_t inb(uint16_t port);
extern "C" void outb(uint16_t port, uint8_t data);

static bool extended = false;

void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    
    // Check for extended scancode prefix
    if (scancode == 0xE0) {
        extended = true;
        outb(0x20, 0x20);
        return;
    }
    
    // Only handle key presses (bit 7 = 0 means key pressed)
    if (scancode & 0x80) {
        extended = false;
        outb(0x20, 0x20);
        return;
    }
    
    // Handle arrow keys (extended scancodes)
    if (extended) {
        extended = false;
        switch (scancode) {
            case 0x48: // Up arrow
                terminal_move_cursor_up();
                break;
            case 0x50: // Down arrow
                terminal_move_cursor_down();
                break;
            case 0x4B: // Left arrow
                terminal_move_cursor_left();
                break;
            case 0x4D: // Right arrow
                terminal_move_cursor_right();
                break;
        }
        outb(0x20, 0x20);
        return;
    }
    
    // Handle regular keys
    if (scancode < sizeof(scancode_to_ascii)) {
        char c = scancode_to_ascii[scancode];
        if (c != 0) {
            terminal_putchar(c);
        }
    }
    
    outb(0x20, 0x20);
}
