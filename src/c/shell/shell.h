#ifndef SHELL_H
#define SHELL_H

#include "../drivers/keyboard/keyboard.h"
#include <stddef.h>

void shell_init();
void shell_keypress(char c);
void shell_keyboard_event_handler(struct keyboard_event event);

#endif