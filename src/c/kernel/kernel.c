#include "kernel.h"
#include "../drivers/vga/vga.h"
#include "../drivers/keyboard/keyboard.h"
#include "../shell/shell.h"

void kernel_main(void) {
    vga_init();
    vga_clear();  // очищаємо залишки BIOS
    vga_print("Kernel has started\n");

    shell_init();

    register_keyboard_interrupt_handler();         // налаштовуємо клавіатуру
    keyboard_set_handler(shell_keyboard_event_handler); // підключаємо shell
}