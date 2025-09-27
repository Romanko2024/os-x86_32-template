#include "kernel/kernel.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/timer/timer.h"
#include "drivers/serial_port/serial_port.h"
#include "shell/shell.h"

//обробник виключень
void exception_handler(u32 interrupt, u32 error, char *message) {
    serial_log(LOG_ERROR, message);
}

//ініціалізація всіх компонентів ядра
void init_kernel_modules() {
    init_gdt();
    init_idt();
    init_exception_handlers();
    init_interrupt_handlers();
    register_timer_interrupt_handler();
    register_keyboard_interrupt_handler();
    configure_default_serial_port();
    set_exception_handler(exception_handler);
    enable_interrupts();
}

//безкінечний цикл для очікування переривань
_Noreturn void halt_loop() {
    while (1) { halt(); }
}

//обробник натискання клавіші
void key_handler(struct keyboard_event event) {
    if (event.key_character && event.type == EVENT_KEY_PRESSED) {
        // shell_keyboard_event_handler обробляє події клавіатури
    }
}

//обробник таймера
void timer_tick_handler() {
    //еееее
}

//func для встановлення курсору у потрібне місце
void put_cursor(unsigned short pos) {
    out(0x3D4, 14);
    out(0x3D5, ((pos >> 8) & 0x00FF));
    out(0x3D4, 15);
    out(0x3D5, pos & 0x00FF);
}

//
void kernel_entry() {
    // 1. ініц ядра
    init_kernel_modules();

    // 2. ініц shell
    shell_init();

    // 3. встановлення обробника клавіатури shell
    keyboard_set_handler(shell_keyboard_event_handler);

    // 4. встановлення обробника таймера (а чи воно щось буде роюити...)
    timer_set_handler(timer_tick_handler);

    // 5. курсор на початок екрану
    put_cursor(0);

    // 6. ЦИКЛ ОЧІКУВАННЯ
    halt_loop();
}
