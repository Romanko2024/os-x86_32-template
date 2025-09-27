#include "kernel.h"
#include "../drivers/vga/vga.h"
#include "../shell/shell.h"

void kernel_main(void) {
    vga_init();
    vga_print("Kernel has started\n");

    shell_init();  // shell буде оброб клавіатурні події через key_handler
    keyboard_set_handler(shell_keypress);
}