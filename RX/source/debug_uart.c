#include "debug_uart.h"

void debug_uart_init(void)
{
    uint16_t sbr;

    /* Enable clocks: PORTA + UART0 */
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;

    /* Select MCGFLLCLK as UART0 clock source */
    SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1);

    /* Configure PTA1 (RX) and PTA2 (TX) for UART0 ALT2 */
    PORTA->PCR[1] = PORT_PCR_MUX(2);   /* UART0_RX */
    PORTA->PCR[2] = PORT_PCR_MUX(2);   /* UART0_TX */

    UART0->C2 = 0;

#define DEBUG_OSR  13u
    sbr = (uint16_t)(DEBUG_CLOCK_HZ / (DEBUG_OSR * DEBUG_BAUD_RATE));
    UART0->BDH = (uint8_t)((sbr >> 8) & 0x1F);
    UART0->BDL = (uint8_t)(sbr & 0xFF);
    UART0->C4  = (UART0->C4 & ~0x1Fu) | (uint8_t)(DEBUG_OSR - 1u);

    UART0->C1 = 0;
    UART0->C2 = UART0_C2_TE_MASK | UART0_C2_RE_MASK;
}

void debug_putchar(char c)
{
    while (!(UART0->S1 & UART0_S1_TDRE_MASK))
        ;
    UART0->D = (uint8_t)c;
}

void debug_puts(const char *s)
{
    while (*s)
        debug_putchar(*s++);
}

void debug_puthex(uint8_t b)
{
    static const char hex[] = "0123456789ABCDEF";
    debug_putchar(hex[b >> 4]);
    debug_putchar(hex[b & 0x0F]);
}

void debug_print_byte(uint8_t c)
{
    if (c >= 0x20 && c <= 0x7E) {
        debug_putchar((char)c);
    } else if (c == '\n') {
        debug_puts("\\n");
    } else if (c == '\r') {
        debug_puts("\\r");
    } else {
        debug_puts("\\x");
        debug_puthex(c);
    }
}
