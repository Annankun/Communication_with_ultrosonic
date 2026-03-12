#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include "MKL25Z4.h"

/*
 * Debug UART0 driver — outputs to OpenSDA virtual COM port on FRDM-KL25Z.
 *
 * Pins:  PTA1 = UART0_RX (ALT2),  PTA2 = UART0_TX (ALT2)
 * Clock: MCGFLLCLK ~20.97 MHz (FEI mode default)
 * Baud:  115200 (fast enough for debug, standard terminal setting)
 */

#define DEBUG_BAUD_RATE  115200u
#define DEBUG_CLOCK_HZ   20971520u   /* MCGFLLCLK in FEI mode */

static inline void debug_uart_init(void)
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

    /* Disable TX/RX while configuring */
    UART0->C2 = 0;

    /*
     * Baud rate with adjusted oversampling ratio (OSR).
     *
     * Default 16x oversampling gives SBR = 20971520/(16*115200) = 11.38
     * which truncates to 11, yielding 3.4% baud error (too high).
     *
     * Using OSR = 13 (register value 12):
     *   SBR = 20971520 / (13 * 115200) = 14.003 → 14
     *   Actual baud = 20971520 / (13 * 14) = 115228 Hz → 0.024% error
     */
    #define DEBUG_OSR  13u
    sbr = (uint16_t)(DEBUG_CLOCK_HZ / (DEBUG_OSR * DEBUG_BAUD_RATE));
    UART0->BDH = (uint8_t)((sbr >> 8) & 0x1F);
    UART0->BDL = (uint8_t)(sbr & 0xFF);
    UART0->C4  = (UART0->C4 & ~0x1Fu) | (uint8_t)(DEBUG_OSR - 1u);

    /* 8-N-1, no parity */
    UART0->C1 = 0;

    /* Enable transmitter and receiver */
    UART0->C2 = UART0_C2_TE_MASK | UART0_C2_RE_MASK;
}

/* Blocking send one byte */
static inline void debug_putchar(char c)
{
    while (!(UART0->S1 & UART0_S1_TDRE_MASK))
        ;
    UART0->D = (uint8_t)c;
}

/* Blocking send a string */
static inline void debug_puts(const char *s)
{
    while (*s)
        debug_putchar(*s++);
}

/* Print one byte as two hex digits */
static inline void debug_puthex(uint8_t b)
{
    static const char hex[] = "0123456789ABCDEF";
    debug_putchar(hex[b >> 4]);
    debug_putchar(hex[b & 0x0F]);
}

/* Print a byte in a human-readable format: printable chars shown as-is,
   control chars shown as \xHH */
static inline void debug_print_byte(uint8_t c)
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

/*
 * PRINTF-style macros for convenience.
 * Use debug_puts() for strings; debug_puthex() for raw byte values.
 */
#define PRINTF  debug_puts

#endif /* DEBUG_UART_H */
