#ifndef TIMER_H
#define TIMER_H

/**
 * Registers timer interrupt handler that delegates handling
 * of the interrupt to the registered interrupt handler (see timer_set_handler).
 */
void register_timer_interrupt_handler();

/**
 * Sets the given function as the handler of the timer interrupts. Calling
 * this function twice will override the previously set function value.
 */
void timer_set_handler(void (*handler)());

/**
 * Resets the idle counter, used by keyboard handler to prevent screensaver
 * from starting while the user is active.
 */
void reset_idle_counter();

#endif
