#include "uart.h"

void uart2_init(void)
{
    uint16_t sbr;

    COMM_UART->C2 = 0;

    /* Baud rate: SBR = BusClock / (16 * baud) */
    sbr = (uint16_t)(BUS_CLOCK_HZ / (16u * COMM_BAUD_RATE));
    COMM_UART->BDH = (uint8_t)((sbr >> 8) & 0x1F);
    COMM_UART->BDL = (uint8_t)(sbr & 0xFF);

    /* 8-N-1, no parity */
    COMM_UART->C1 = 0;

    /* Enable transmitter and receiver */
    COMM_UART->C2 = UART_C2_TE_MASK | UART_C2_RE_MASK;
}

/* Blocking send one byte */
void uart2_putchar(uint8_t c)
{
    while (!(COMM_UART->S1 & UART_S1_TDRE_MASK))
        ;
    COMM_UART->D = c;
}

/* Blocking send a string */
void uart2_puts(const char *s)
{
    while (*s)
        uart2_putchar((uint8_t)*s++);
}

/* Non-blocking receive: returns 1 if byte available, 0 otherwise */
int uart2_getchar(uint8_t *out)
{
    if (COMM_UART->S1 & UART_S1_RDRF_MASK) {
        *out = COMM_UART->D;
        return 1;
    }
    return 0;
}
