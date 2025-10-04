#include "screensaver.h"
#include "../../drivers/vga/vga.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/timer/timer.h"
#include "../../shell/shell.h"
#include <stdint.h>

// extern змінні з VGA-драйвера
extern uint16_t* vga_buffer;
extern uint8_t cursor_x;
extern uint8_t cursor_y;

//таймерний обробник
extern void timer_tick_handler();

//обробник клавіатури
extern void shell_keyboard_event_handler(struct keyboard_event event);

// --- збереження стану ---
static uint16_t saved_buffer[80 * 25];
static uint8_t saved_cursor_x;
static uint8_t saved_cursor_y;

// ---
static int active = 0;
static int anim_x = 0;
static int anim_y = 0;
static int anim_dx = 1;
static int anim_dy = 1;
static const char anim_char = '*';

// таймер функ.
void screensaver_timer_handler() {
    // очищення екрана
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_put_char_at(' ', y, x);
        }
    }

    //рух символу
    anim_x += anim_dx;
    anim_y += anim_dy;

    if (anim_x <= 0) { anim_x = 0; anim_dx = 1; }
    if (anim_x >= VGA_WIDTH - 1) { anim_x = VGA_WIDTH - 1; anim_dx = -1; }
    if (anim_y <= 0) { anim_y = 0; anim_dy = 1; }
    if (anim_y >= VGA_HEIGHT - 1) { anim_y = VGA_HEIGHT - 1; anim_dy = -1; }

    //відображення символу
    vga_put_char_at(anim_char, anim_y, anim_x);

    // прибирання курсора
    vga_set_cursor(VGA_HEIGHT - 1, 0);
}

// обробник клавіатури під screensaver
void screensaver_key_handler(struct keyboard_event event) {
    if (event.type == EVENT_KEY_PRESSED) {
        screensaver_stop();
    }
}
// запуск screensaver
void screensaver_start() {
    if (active) return;

    // зберігання VGA буфера і поз. курсора
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        saved_buffer[i] = vga_buffer[i];
    }
    saved_cursor_x = cursor_x;
    saved_cursor_y = cursor_y;

    // очищення екрана
    vga_clear();

    // ініц положення символу
    anim_x = VGA_WIDTH / 2;
    anim_y = VGA_HEIGHT / 2;
    anim_dx = 1;
    anim_dy = 1;

    // заміна обробника клавіатури
    keyboard_set_handler(screensaver_key_handler);

    // таймер
    timer_set_handler(screensaver_timer_handler);

    active = 1;
}

// зупинка screensaver і відновлення 
void screensaver_stop() {
    if (!active) return;

    //відновлення VGA буфера
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = saved_buffer[i];
    }

    //відновлення позиції курсора
    vga_set_cursor(saved_cursor_y, saved_cursor_x);

    //відновлення обробників клавіатури та таймера
    keyboard_set_handler(shell_keyboard_event_handler);
    timer_set_handler(timer_tick_handler);

    active = 0;
}

// статус чек
int screensaver_is_active() {
    return active;
}
