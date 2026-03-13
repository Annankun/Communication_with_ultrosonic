#ifndef UART_H
#define UART_H

#include "MKL25Z4.h"
#include "pin_config.h"

#define BUS_CLOCK_HZ  10485760u   /* Default FEI bus clock */

void uart2_init(void);
void uart2_putchar(uint8_t c);
void uart2_puts(const char *s);
int uart2_getchar(uint8_t *out);

#endif /* UART_H */
