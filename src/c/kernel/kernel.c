#include "kernel.h"
#include "../drivers/vga/vga.h"
#include "../shell/shell.h"

void kernel_main(void) {
    vga_init();  // ініц екрана
    vga_print("Kernel has started\n");

    shell_init();
    shell_run();
}