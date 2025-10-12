#include "shell.h"
#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/timer/timer.h"
#include "../fs/fs.h"
#include "../utils/mem.h"

// ------------------ Система команд ------------------

typedef void (*CommandHandler)(const char* args);

typedef struct {
    const char* name;
    CommandHandler handler;
} Command;

#define MAX_COMMANDS 20
static Command commands[MAX_COMMANDS];
static int command_count = 0;

// буфер введення
static char input_buffer[128];
static int input_pos = 0;

// Локальні змінні для курсора (щоб не чіпати VGA-драйвер)
static int shell_cursor_row = 0;
static int shell_cursor_col = 0;

// ------------------ Реєстрація команд ------------------

void register_command(const char* name, CommandHandler handler) {
    if (command_count < MAX_COMMANDS) {
        commands[command_count].name = name;
        commands[command_count].handler = handler;
        command_count++;
    }
}

// ------------------ Реалізації команд ------------------

static void cmd_help(const char* args);
static void cmd_clear(const char* args);
static void cmd_list(const char* args);
static void cmd_create(const char* args);
static void cmd_write(const char* args);
static void cmd_read(const char* args);
static void cmd_delete(const char* args);

// ------------------ Ініціалізація ------------------

void shell_init() {
    vga_clear();
    fs_init();

    shell_cursor_row = 0;
    shell_cursor_col = 0;

    register_command("help", cmd_help);
    register_command("clear", cmd_clear);
    register_command("list", cmd_list);
    register_command("create", cmd_create);
    register_command("write", cmd_write);
    register_command("read", cmd_read);
    register_command("delete", cmd_delete);

    vga_println("Simple shell initialized. Type 'help' for commands.");
    keyboard_set_handler(shell_keyboard_event_handler);
}

// ------------------ Обробка клавіш ------------------

void shell_backspace() {
    extern uint8_t cursor_x;
    extern uint8_t cursor_y;

    if (cursor_x == 0 && cursor_y == 0) {
        return;
    }

    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = VGA_WIDTH - 1;
    }

    vga_put_char_at(' ', cursor_y, cursor_x);
    vga_set_cursor(cursor_y, cursor_x);
}

void shell_keypress(char c) {
    if (c == '\b') { // backspace
        shell_backspace();
        return;
    }

    if (c == '\n') { // enter
        input_buffer[input_pos] = '\0';
        vga_putc('\n');
        execute_command(input_buffer);
        input_pos = 0;
        shell_cursor_row++;
        shell_cursor_col = 0;
        return;
    }

    if (input_pos < (int)(sizeof(input_buffer) - 1)) {
        input_buffer[input_pos++] = c;
        vga_putc(c);

        shell_cursor_col++;
        if (shell_cursor_col >= VGA_WIDTH) {
            shell_cursor_col = 0;
            shell_cursor_row++;
        }
    }
}

// ------------------ Обробник подій клавіатури ------------------

void shell_keyboard_event_handler(struct keyboard_event event) {
    if (event.type == EVENT_KEY_PRESSED) {
        char c = event.key_character;
        if (c) shell_keypress(c);
    }
}

// ------------------ Виконання команд ------------------

void execute_command(const char* input) {
    char cmd[64];
    const char* args = strchr(input, ' ');
    if (args) {
        size_t len = args - input;
        strncpy(cmd, input, len);
        cmd[len] = '\0';
        args++;
    } else {
        strcpy(cmd, input);
        args = "";
    }

    for (int i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, cmd) == 0) {
            commands[i].handler(args);
            return;
        }
    }
    vga_println("Unknown command");
}

// ------------------ Команди ------------------

static void cmd_help(const char* args) {
    (void)args;
    vga_println("Available commands:");
    vga_println("  help           - show this message");
    vga_println("  clear          - clear the screen");
    vga_println("  create <name>  - create file");
    vga_println("  write <name> <text> - append text to file");
    vga_println("  read <name>    - display file content");
    vga_println("  delete <name>  - delete file");
    vga_println("  list           - list all files");
}

static void cmd_clear(const char* args) {
    (void)args;
    vga_clear();
    shell_cursor_row = 0;
    shell_cursor_col = 0;
}

static void cmd_list(const char* args) {
    (void)args;
    fs_list();
}

static void cmd_create(const char* args) {
    if (strlen(args) == 0) {
        vga_println("Usage: create <filename>");
        return;
    }
    if (fs_create(args))
        vga_println("File created.");
    else
        vga_println("No space or file exists.");
}

static void cmd_write(const char* args) {
    char name[32];
    const char* text = strchr(args, ' ');
    if (!text) {
        vga_println("Usage: write <filename> <text>");
        return;
    }
    size_t len = text - args;
    strncpy(name, args, len);
    name[len] = '\0';
    text++;

    if (fs_write(name, text))
        vga_println("Written.");
    else
        vga_println("File not found.");
}

static void cmd_read(const char* args) {
    const char* content = fs_read(args);
    if (content)
        vga_println(content);
    else
        vga_println("File not found.");
}

static void cmd_delete(const char* args) {
    if (fs_delete(args))
        vga_println("File deleted.");
    else
        vga_println("File not found.");
}
