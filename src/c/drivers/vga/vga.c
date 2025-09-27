#include "vga.h"

uint16_t* vga_buffer = (uint16_t*)VGA_ADDRESS;
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

//символ у форматі VGA (char + атрибути)
static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

//колір
static uint8_t default_color = 0x07;

void vga_init() {
    vga_clear();
}

void vga_clear() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', default_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_scroll() {
    for (int y = 1; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];

    for (int x = 0; x < VGA_WIDTH; x++)
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', default_color);

    if (cursor_y > 0) cursor_y--;
}

void vga_putc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, default_color);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    vga_scroll();
}

void vga_print(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}
void vga_set_cursor(int row, int col) {
    cursor_x = col;
    cursor_y = row;
}

void vga_put_char_at(char c, int row, int col) {
    if (row < 0 || row >= VGA_HEIGHT || col < 0 || col >= VGA_WIDTH) return;
    vga_buffer[row * VGA_WIDTH + col] = vga_entry(c, 0x07);
}

void vga_copy_char(int src_row, int src_col, int dst_row, int dst_col) {
    if (src_row < 0 || src_row >= VGA_HEIGHT || src_col < 0 || src_col >= VGA_WIDTH) return;
    if (dst_row < 0 || dst_row >= VGA_HEIGHT || dst_col < 0 || dst_col >= VGA_WIDTH) return;
    vga_buffer[dst_row * VGA_WIDTH + dst_col] =
        vga_buffer[src_row * VGA_WIDTH + src_col];
}

void vga_puts(const char* str) {
    while (*str) vga_putc(*str++);
}

void vga_put_char_at_color(char c, int row, int col, uint8_t color) {
    if (row < 0 || row >= VGA_HEIGHT || col < 0 || col >= VGA_WIDTH) return;
    vga_buffer[row * VGA_WIDTH + col] = vga_entry(c, color);
}

