#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "MKL25Z4.h"

#define RGB_RED_PORT    PORTB
#define RGB_RED_GPIO    GPIOB
#define RGB_RED_PIN     18      /* PTB18 - TPM2_CH0 */

#define RGB_GREEN_PORT  PORTB
#define RGB_GREEN_GPIO  GPIOB
#define RGB_GREEN_PIN   19      /* PTB19 - TPM2_CH1 */

#define RGB_BLUE_PORT   PORTD
#define RGB_BLUE_GPIO   GPIOD
#define RGB_BLUE_PIN    1       /* PTD1  - TPM0_CH1 */

#define COMM_UART           UART2
#define COMM_UART_IRQn      UART2_IRQn
#define COMM_UART_IRQHandler UART2_IRQHandler

#define COMM_TX_PORT    PORTD
#define COMM_TX_PIN     3       /* PTD3 - UART2_TX (ALT3) */

#define COMM_RX_PORT    PORTD
#define COMM_RX_PIN     2       /* PTD2 - UART2_RX (ALT3) */

#define COMM_UART_ALT   3       /* Port mux ALT3 for UART2 */
#define COMM_BAUD_RATE  9600

#define IR_OBS_COUNT    6u

void pin_config_init(void);

#define RGB_RED_ON()     (RGB_RED_GPIO->PCOR   = (1u << RGB_RED_PIN))
#define RGB_RED_OFF()    (RGB_RED_GPIO->PSOR   = (1u << RGB_RED_PIN))
#define RGB_GREEN_ON()   (RGB_GREEN_GPIO->PCOR = (1u << RGB_GREEN_PIN))
#define RGB_GREEN_OFF()  (RGB_GREEN_GPIO->PSOR = (1u << RGB_GREEN_PIN))
#define RGB_BLUE_ON()    (RGB_BLUE_GPIO->PCOR  = (1u << RGB_BLUE_PIN))
#define RGB_BLUE_OFF()   (RGB_BLUE_GPIO->PSOR  = (1u << RGB_BLUE_PIN))

#define RGB_ALL_OFF() do { \
    RGB_RED_OFF(); RGB_GREEN_OFF(); RGB_BLUE_OFF(); \
} while (0)

#endif /* PIN_CONFIG_H */
