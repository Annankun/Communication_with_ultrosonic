#ifndef UART_H
#define UART_H

#include "MKL25Z4.h"
#include "pin_config.h"

/*
 * UART2 polling driver for Step 1 (byte-level connectivity).
 *
 * Bus clock assumed ~20.97 MHz (default FEI mode on FRDM-KL25Z).
 * Will be replaced by ISR + ring-buffer in Step 2.
 */

#define BUS_CLOCK_HZ  10485760u   /* Default FEI bus clock */

void uart2_init(void);
void uart2_putchar(uint8_t c);
void uart2_puts(const char *s);
int uart2_getchar(uint8_t *out);

#endif /* UART_H */
