#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <stdint.h>

// Запускає screensaver (анімований). Повертає 0 якщо старт успішний.
void screensaver_start();

// Зупиняє screensaver і відновлює попередній стан.
void screensaver_stop();

// Чи активний screensaver?
int screensaver_is_active();

#endif