#include "pin_config.h"

void pin_config_init(void)
{
    /* Enable clock gating for Port B, Port D, and Port E */
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK
                | SIM_SCGC5_PORTE_MASK;

    /* --- RGB LED pins: GPIO output --- */
    RGB_RED_PORT->PCR[RGB_RED_PIN]   = PORT_PCR_MUX(1);
    RGB_GREEN_PORT->PCR[RGB_GREEN_PIN] = PORT_PCR_MUX(1);
    RGB_BLUE_PORT->PCR[RGB_BLUE_PIN] = PORT_PCR_MUX(1);

    RGB_RED_GPIO->PDDR   |= (1u << RGB_RED_PIN);
    RGB_GREEN_GPIO->PDDR |= (1u << RGB_GREEN_PIN);
    RGB_BLUE_GPIO->PDDR  |= (1u << RGB_BLUE_PIN);

    /* Start with all LEDs off (active-low: set high = off) */
    RGB_RED_GPIO->PSOR   = (1u << RGB_RED_PIN);
    RGB_GREEN_GPIO->PSOR = (1u << RGB_GREEN_PIN);
    RGB_BLUE_GPIO->PSOR  = (1u << RGB_BLUE_PIN);

    /* --- UART2 pins: ALT4 mux --- */
    COMM_TX_PORT->PCR[COMM_TX_PIN] = PORT_PCR_MUX(COMM_UART_ALT);

    /* Enable UART2 clock */
    SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
}
