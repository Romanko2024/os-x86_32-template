#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

//поточні коордикурсора
extern uint8_t cursor_x;
extern uint8_t cursor_y;

// функції для роботи з VGA
void vga_init();
void vga_putc(char c);
void vga_print(const char* str);
void vga_clear();
void vga_scroll();
void vga_set_cursor(int row, int col);
void vga_put_char_at(char c, int row, int col);
void vga_copy_char(int src_row, int src_col, int dst_row, int dst_col);
void vga_puts(const char* str);
#endif
