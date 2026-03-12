#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include "MKL25Z4.h"

#define DEBUG_BAUD_RATE  115200u
#define DEBUG_CLOCK_HZ   20971520u   /* MCGFLLCLK in FEI mode */

void debug_uart_init(void);
void debug_putchar(char c);
void debug_puts(const char *s);
void debug_puthex(uint8_t b);
void debug_print_byte(uint8_t c);

#define PRINTF  debug_puts

#endif /* DEBUG_UART_H */
