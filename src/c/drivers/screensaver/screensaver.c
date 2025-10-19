#include "screensaver.h"
#include "../../drivers/vga/vga.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/timer/timer.h"
#include "../../shell/shell.h"
#include <stdint.h>

extern uint16_t* vga_buffer;
extern uint8_t cursor_x;
extern uint8_t cursor_y;
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

// --- час і перемикання ---
static int frame_counter = 0;
static int current_pattern = 0;
#define SWITCH_INTERVAL 90

typedef struct {
    int dx;
    int dy;
    char c;
} StarPart;

// ====== Візерунки ======

static const StarPart pattern_comet[] = {
    {0, 0, '*'},
    {-1, 0, '*'},
    {1, 0, '*'},
    {0, -1, '*'},
    {0, 1, '*'},
    {-1, -1, '.'},
    {1, 1, '.'},
    {-1, 1, '.'},
    {1, -1, '.'}
};

static const StarPart pattern_box[] = {
    {-1, -1, '*'}, {0, -1, '*'}, {1, -1, '*'},
    {-1, 0, '*'},                {1, 0, '*'},
    {-1, 1, '*'},  {0, 1, '*'},  {1, 1, '*'}
};

static const StarPart pattern_cross[] = {
    {0, 0, '*'},
    {0, -1, '*'}, {0, 1, '*'},
    {-1, 0, '*'}, {1, 0, '*'},
    {0, -2, '.'}, {0, 2, '.'},
    {-2, 0, '.'}, {2, 0, '.'}
};

typedef struct {
    const StarPart* shape;
    int len;
} Pattern;

static const Pattern patterns[] = {
    {pattern_comet, sizeof(pattern_comet) / sizeof(StarPart)},
    {pattern_box, sizeof(pattern_box) / sizeof(StarPart)},
    {pattern_cross, sizeof(pattern_cross) / sizeof(StarPart)}
};
#define PATTERN_COUNT (sizeof(patterns) / sizeof(Pattern))

// ====== малювання ======
static void draw_star(int y, int x) {
    const Pattern* p = &patterns[current_pattern];
    for (int i = 0; i < p->len; i++) {
        int sx = x + p->shape[i].dx;
        int sy = y + p->shape[i].dy;
        if (sx >= 0 && sx < VGA_WIDTH && sy >= 0 && sy < VGA_HEIGHT) {
            vga_put_char_at(p->shape[i].c, sy, sx);
        }
    }
}

static void clear_star(int y, int x) {
    const Pattern* p = &patterns[current_pattern];
    for (int i = 0; i < p->len; i++) {
        int sx = x + p->shape[i].dx;
        int sy = y + p->shape[i].dy;
        if (sx >= 0 && sx < VGA_WIDTH && sy >= 0 && sy < VGA_HEIGHT) {
            vga_put_char_at(' ', sy, sx);
        }
    }
}

void screensaver_timer_handler() {
    // очищення попереднього кадру
    clear_star(anim_y, anim_x);

    // рух символу
    anim_x += anim_dx;
    anim_y += anim_dy;

    if (anim_x <= 1) { anim_x = 1; anim_dx = 1; }
    if (anim_x >= VGA_WIDTH - 2) { anim_x = VGA_WIDTH - 2; anim_dx = -1; }
    if (anim_y <= 1) { anim_y = 1; anim_dy = 1; }
    if (anim_y >= VGA_HEIGHT - 2) { anim_y = VGA_HEIGHT - 2; anim_dy = -1; }

    //малювання нового кадру
    draw_star(anim_y, anim_x);

    // лічильник кадрів
    frame_counter++;
    if (frame_counter >= SWITCH_INTERVAL) {
        frame_counter = 0;
        current_pattern = (current_pattern + 1) % PATTERN_COUNT;
    }

    vga_set_cursor(VGA_HEIGHT - 1, 0);
}

void screensaver_key_handler(struct keyboard_event event) {
    if (event.type == EVENT_KEY_PRESSED) {
        screensaver_stop();
    }
}

void screensaver_start() {
    if (active) return;

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        saved_buffer[i] = vga_buffer[i];
    }
    saved_cursor_x = cursor_x;
    saved_cursor_y = cursor_y;

    vga_clear();

    anim_x = VGA_WIDTH / 2;
    anim_y = VGA_HEIGHT / 2;
    anim_dx = 1;
    anim_dy = 1;
    frame_counter = 0;
    current_pattern = 0;

    keyboard_set_handler(screensaver_key_handler);
    timer_set_handler(screensaver_timer_handler);

    active = 1;
}

void screensaver_stop() {
    if (!active) return;

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = saved_buffer[i];
    }

    vga_set_cursor(saved_cursor_y, saved_cursor_x);

    keyboard_set_handler(shell_keyboard_event_handler);
    timer_set_handler(timer_tick_handler);

    active = 0;
}

int screensaver_is_active() {
    return active;
}
