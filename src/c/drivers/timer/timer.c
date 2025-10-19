#include "../../kernel/kernel.h"
#include "timer.h"
#include "../../drivers/screensaver/screensaver.h"

static int idle_counter = 0;

void (*custom_timer_interrupt_handler)() = 0;

void timer_handler(__attribute__((unused)) u32 interrupt) {
    if (custom_timer_interrupt_handler != 0) {
        custom_timer_interrupt_handler();
    }

    // лічильник бездіяльності для автозапуску screensaver
    if (!screensaver_is_active()) {
        idle_counter++;
        if (idle_counter >= 240) { // 8 секунд * 30 тік
            screensaver_start();
        }
    }
}

// функція скидання лічильника бездіяльності
void reset_idle_counter() {
    idle_counter = 0;
}

void register_timer_interrupt_handler() {
    set_interrupt_handler(INTERRUPT_TIMER, timer_handler);
}

void timer_set_handler(void (*handler)()) {
    custom_timer_interrupt_handler = handler;
}


