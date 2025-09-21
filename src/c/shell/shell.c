#include "../../vga/vga.h"   // шлях до vga.h а де воно? а воно э?.....
#include "shell.h" //ну тут хоч норм
#include <string.h> //ееее

#define CMD_MAX_LEN 80
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

char cmd_buffer[CMD_MAX_LEN];
int cmd_len = 0;

// Позиція курсора
int cursor_row = 0;
int cursor_col = 0;

// Прототипи
void execute_command(const char *cmd);
void scroll_screen();
void shell_put_char(char c);
void shell_newline();
void shell_backspace();

// Ініціалізація shell
void shell_init() {
    vga_clear();
    cursor_row = 0;
    cursor_col = 0;
    cmd_len = 0;
    memset(cmd_buffer, 0, CMD_MAX_LEN);
    vga_set_cursor(cursor_row, cursor_col);
}

// Виклик при натисканні клавіші
void shell_keypress(char c) {
    if (c == '\n') { // Enter
        shell_newline();
        cmd_buffer[cmd_len] = 0; // null-terminate
        execute_command(cmd_buffer);
        cmd_len = 0;
        memset(cmd_buffer, 0, CMD_MAX_LEN);
    } 
    else if (c == '\b') { // Backspace
        shell_backspace();
    } 
    else { // Звичайний символ
        if (cmd_len < CMD_MAX_LEN - 1) {
            cmd_buffer[cmd_len++] = c;
            shell_put_char(c);
        }
    }
}

// Вивід символу на екран
void shell_put_char(char c) {
    vga_put_char_at(c, cursor_row, cursor_col);
    cursor_col++;
    if (cursor_col >= SCREEN_COLS) {
        shell_newline();
    }
    vga_set_cursor(cursor_row, cursor_col);
}

// Перехід на новий рядок
void shell_newline() {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= SCREEN_ROWS) {
        scroll_screen();
        cursor_row = SCREEN_ROWS - 1;
    }
    vga_set_cursor(cursor_row, cursor_col);
}

// Видалення символу
void shell_backspace() {
    if (cmd_len > 0) {
        cmd_len--;
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = SCREEN_COLS - 1;
        }
        vga_put_char_at(' ', cursor_row, cursor_col);
        vga_set_cursor(cursor_row, cursor_col);
    }
}

// Скролл екрану вгору
void scroll_screen() {
    for (int r = 1; r < SCREEN_ROWS; r++) {
        for (int c = 0; c < SCREEN_COLS; c++) {
            vga_copy_char(r, c, r - 1, c);
        }
    }
    // Очистити останній рядок
    for (int c = 0; c < SCREEN_COLS; c++) {
        vga_put_char_at(' ', SCREEN_ROWS - 1, c);
    }
}

// Виконання команди
void execute_command(const char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        vga_puts("Available commands: help, clear\n");
    } else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
        cursor_row = 0;
        cursor_col = 0;
    } else if (strlen(cmd) == 0) {
        // нічого не робимо
    } else {
        vga_puts("Unknown command\n");
    }
    shell_newline();
}