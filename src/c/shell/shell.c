#include "shell.h"
#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/timer/timer.h"
#include "../fs/fs.h"
#include "../utils/mem.h"
#include "../drivers/screensaver/screensaver.h"

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

// ------------------ Редактор ------------------
static int editing = 0;            // чи в режимі редагування
static File* edit_file = NULL;     // файл який редагується (вказівник від fs_get)
static int edit_offset = 0;        // позиція курсору в content (0..len)
static char edit_tmp[MAX_CONTENT_SIZE]; // тимчасовий буфер (ми будемо редагувати тут)
static int edit_len = 0;           // поточна довжина в edit_tmp

// ------------------ Допоміжні функції ------------------
static void shell_show_prompt() {
    vga_print("$ ");
    input_pos = 0; // скидаємо буфер для нового вводу
}

// перепоз. апаратний курсор з edit_offset
static void editor_update_cursor_pos() {
    int row = edit_offset / VGA_WIDTH;
    int col = edit_offset % VGA_WIDTH;
    if (row >= VGA_HEIGHT) row = VGA_HEIGHT - 1;
    if (col >= VGA_WIDTH) col = VGA_WIDTH - 1;
    vga_set_cursor(row, col);
}

// переписати весь вміст редактора з edit_tmp
static void editor_redraw() {
    vga_clear();
    // вивести рядками
    int i = 0;
    for ( ; i < edit_len; i++) {
        vga_putc(edit_tmp[i]);
    }
    // якщо останній символ — не newline, то курсор  right after last printed char
    editor_update_cursor_pos();
}

//завершення редагування (зберегти зміни у fs, повернути обробник)
static void editor_finish_and_save() {
    if (!editing || !edit_file) return;
    // зберегти у fs
    fs_save_content(edit_file->name, edit_tmp);
    // повернутися до shell режиму
    editing = 0;
    edit_file = NULL;
    edit_offset = 0;
    edit_len = 0;
    // очистити та показати промт
    vga_clear();
    shell_show_prompt();
    // відновити обробник клавіатури shell
    keyboard_set_handler(shell_keyboard_event_handler);
}

// відміна редагування без збереження (поки не реалізовано)
static void editor_abort_without_saving() {
    editing = 0;
    edit_file = NULL;
    edit_offset = 0;
    edit_len = 0;
    vga_clear();
    shell_show_prompt();
    keyboard_set_handler(shell_keyboard_event_handler);
}

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
static void cmd_screensaver(const char* args);
static void cmd_edit(const char* args);

// ------------------ Ініціалізація ------------------

void shell_init() {
    vga_clear();
    fs_init();

    register_command("help", cmd_help);
    register_command("clear", cmd_clear);
    register_command("list", cmd_list);
    register_command("create", cmd_create);
    register_command("write", cmd_write);
    register_command("read", cmd_read);
    register_command("delete", cmd_delete);
    register_command("screensaver", cmd_screensaver);
    register_command("edit", cmd_edit);

    vga_println("Simple shell initialized. Type 'help' for commands.");
    shell_show_prompt();

    keyboard_set_handler(shell_keyboard_event_handler);
}

// ------------------ Обробка клавіш для shell ------------------

void shell_backspace() {
    extern uint8_t cursor_x;
    extern uint8_t cursor_y;

    // якщо немає введених символів — не дозволяємо стирати промт
    if (input_pos <= 0)
        return;
    // видаляємо останній символ
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = VGA_WIDTH - 1;
    }

    vga_put_char_at(' ', cursor_y, cursor_x);
    vga_set_cursor(cursor_y, cursor_x);
    input_pos--;
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
        shell_show_prompt();
        return;
    }

    if (input_pos < (int)(sizeof(input_buffer) - 1)) {
        input_buffer[input_pos++] = c;
        vga_putc(c);
    }
}

// ------------------ Обробник подій клавіатури (перенаправляє в shell або editor) ------------------

void shell_keyboard_event_handler(struct keyboard_event event) {
    if (event.type != EVENT_KEY_PRESSED) return;

    if (editing) {
        // у режимі редактора обробляє інший обробник (ми використовуємо editor handler directly)
        return;
    }

    // shell режим
    char c = event.key_character;
    if (c) shell_keypress(c);
}

// ------------------ Обробник клавіатури для редактора ------------------

void editor_keyboard_event_handler(struct keyboard_event event) {
    if (event.type != EVENT_KEY_PRESSED) return;

    //if натиснута ESC — зберегти і вийти
    if (event.key == KEY_ESC) {
        editor_finish_and_save();
        // відновлення клаватурного обробника вже зроблено всередині editor_finish_and_save
        return;
    }

    //navigation
    // '[' - вліво, ']' - вправо, ',' - вгору, '.' - вниз
    if (event.key == KEY_LEFT_RECTANGLE_BRACKET) {
        if (edit_offset > 0) edit_offset--;
        editor_update_cursor_pos();
        return;
    }
    if (event.key == KEY_RIGHT_RECTANGLE_BRACKET) {
        if (edit_offset < edit_len) edit_offset++;
        editor_update_cursor_pos();
        return;
    }
    if (event.key == KEY_COMMA) {
        // up
        if (edit_offset >= VGA_WIDTH) edit_offset -= VGA_WIDTH;
        else edit_offset = 0;
        editor_update_cursor_pos();
        return;
    }
    if (event.key == KEY_DOT) {
        // down
        if (edit_offset + VGA_WIDTH <= edit_len) edit_offset += VGA_WIDTH;
        else edit_offset = edit_len;
        editor_update_cursor_pos();
        return;
    }

    // Backspace
    if (event.key == KEY_BACKSPACE) {
        if (edit_offset > 0) {
            // зсунути все зліва на одну позицію вліво
            for (int i = edit_offset - 1; i < edit_len - 1; i++) {
                edit_tmp[i] = edit_tmp[i + 1];
            }
            edit_len--;
            edit_tmp[edit_len] = '\0';
            edit_offset--;
            editor_redraw();
        }
        return;
    }

    // Enter (вставляємо '\n')
    if (event.key == KEY_ENTER) {
        if (edit_len + 1 < MAX_CONTENT_SIZE) {
            // shift right від edit_offset, вставити '\n'
            for (int i = edit_len; i >= edit_offset; i--) {
                edit_tmp[i + 1] = edit_tmp[i];
            }
            edit_tmp[edit_offset] = '\n';
            edit_len++;
            edit_offset++;
            editor_redraw();
        }
        return;
    }

    // вставка звичайного символа (якщо є key_character і є місце)
    if (event.key_character && edit_len + 1 < MAX_CONTENT_SIZE) {
        char ch = event.key_character;
        // вставити ch в edit_tmp на позицію edit_offset
        for (int i = edit_len; i >= edit_offset; i--) {
            edit_tmp[i + 1] = edit_tmp[i];
        }
        edit_tmp[edit_offset] = ch;
        edit_len++;
        edit_offset++;
        editor_redraw();
        return;
    }

    // інші клавіші — ігнор
}

// ------------------ Режим редагування: старт ------------------

static void start_edit_mode(File* f) {
    if (!f) return;
    editing = 1;
    edit_file = f;
    // копіюємо вміст у тимчасовий буфер
    size_t i = 0;
    for (; i < MAX_CONTENT_SIZE - 1 && f->content[i] != '\0'; i++) {
        edit_tmp[i] = f->content[i];
    }
    edit_len = (int)i;
    edit_tmp[edit_len] = '\0';
    edit_offset = edit_len; // починати в кінці

    // показати файл на екрані
    editor_redraw();

    // підмінити обробник клавіатури на редакторський
    keyboard_set_handler(editor_keyboard_event_handler);
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
    vga_println("  screensaver");
    vga_println("  edit <name>    - edit file (use [ ] , . for nav, Esc to save & exit)");
}

static void cmd_clear(const char* args) {
    (void)args;
    vga_clear();
    shell_show_prompt();
}

static void cmd_screensaver(const char* args) {
    (void)args;
    screensaver_start();
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

    int result = fs_create(args);

    if (result == 1)
        vga_println("File created.");
    else if (result == -1)
        vga_println("File already exists.");
    else
        vga_println("No space left.");
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

// команда edit
static void cmd_edit(const char* args) {
    if (strlen(args) == 0) {
        vga_println("Usage: edit <filename>");
        return;
    }
    File* f = fs_get(args);
    if (!f) {
        vga_println("File not found.");
        return;
    }
    // стартуємо режим редагування
    start_edit_mode(f);
}

