#include "../drivers/vga/vga.h"
#include "shell.h" //ну тут хоч норм
#include "../utils/mem.h"
#include "../drivers/keyboard/keyboard.h"

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
    vga_set_cursor(cursor_row, cursor_col);  //курсор у верхній лівий кут
    cmd_len = 0;
    memset(cmd_buffer, 0, CMD_MAX_LEN);
}

// Виклик при натисканні клавіші
void shell_keypress(char c) {
    if (c == '\n') { // Enter
        shell_newline();
        cmd_buffer[cmd_len] = 0; // null-terminate
        execute_command(cmd_buffer);
        cmd_len = 0;
        memset(cmd_buffer, 0, CMD_MAX_LEN);
    } else if (c == '\b') { // Backspace
        shell_backspace();
    } else { // звичайний символ
        if (cmd_len < CMD_MAX_LEN - 1) {
            cmd_buffer[cmd_len++] = c;
            shell_put_char(c);
        }
    }
}

// Вивід символу на екран
void shell_put_char(char c) {
    vga_put_char_at(c, cursor_row, cursor_col);  // <-- використано правильну функцію
    cursor_col++;
    if (cursor_col >= SCREEN_COLS) shell_newline();
}

// Перехід на новий рядок
void shell_newline() {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= SCREEN_ROWS) scroll_screen();
}

// Видалення символу
void shell_backspace() {
    if (cmd_len > 0) {
        cmd_len--;
        if (cursor_col > 0) cursor_col--;
        else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = SCREEN_COLS - 1;
        }
        vga_put_char_at(' ', cursor_row, cursor_col);  // <-- правильна функція
    }
}

// Скролл екрану
void scroll_screen() {
    vga_scroll();
    if (cursor_row > 0) cursor_row = SCREEN_ROWS - 1;
}

// Виконання команди
void execute_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        vga_print("Available commands: help, clear\n");
    } else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
        cursor_row = 0;
        cursor_col = 0;
    } else if (strlen(cmd) == 0) {
        // нічого не робимо
    } else {
        vga_print("Unknown command\n");
    }
    shell_newline();
}
// Адаптер для keyboard_event
void shell_keyboard_event_handler(struct keyboard_event event) {
    if (event.type == EVENT_KEY_PRESSED) {
        shell_keypress(event.key_character);
    }
}