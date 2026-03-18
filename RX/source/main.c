

#include "MKL25Z4.h"
#include "pin_config.h"
#include "uart.h"
#include "debug_uart.h"
#include "ringbuf.h"
#include "protocol.h"
#include "sensor_status.h"

/* Keep one definition for shared headers. */
sensor_status_t g_sensor_status;

ringbuf_t         rx_ring;
volatile uint32_t rx_overflow_count;
volatile uint32_t hw_overrun_count;

void UART2_IRQHandler(void)
{
    uint8_t status = COMM_UART->S1;

    if (status & UART_S1_RDRF_MASK) {
        uint8_t c = COMM_UART->D;
        if (!ring_push(&rx_ring, c))
            rx_overflow_count++;
    }

    if (status & UART_S1_OR_MASK) {
        (void)COMM_UART->D;
        hw_overrun_count++;
    }
}

/* ---- SysTick: 1 ms ---- */

static volatile uint32_t ms_ticks;

void SysTick_Handler(void) { ms_ticks++; }

static void delay_ms(uint32_t ms)
{
    uint32_t start = ms_ticks;
    while ((ms_ticks - start) < ms)
        ;
}


static void debug_print_rx(uint8_t us_priority)
{
    PRINTF("[RX] US_PRI=");
    debug_putchar((char)('0' + us_priority));
    PRINTF("\r\n");
}

int main(void)
{
    parser_t   parser;
    uint8_t    c;
    uint8_t    debug_ctr  = 0;
    uint8_t    us_priority = 0u;

    uint32_t last_valid_rx = 0;
    uint8_t  first_frame   = 1;
    uint8_t  in_safe_mode  = 0;

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000u);

    pin_config_init();
    ring_init(&rx_ring);
    rx_overflow_count = 0;
    hw_overrun_count = 0;
    uart2_init();
    debug_uart_init();

    RGB_ALL_OFF();
    parser_init(&parser);

    /* Startup blink: 3x blue */
    for (int b = 0; b < 3; b++) {
        RGB_BLUE_ON();  delay_ms(150);
        RGB_BLUE_OFF(); delay_ms(150);
    }

    while (1) {
        /* ---- 1. Process incoming bytes ---- */
        while (uart2_getchar(&c)) {
            int result = parser_feed(&parser, c);

            if (result == PARSE_OK) {
                last_valid_rx = ms_ticks;
                first_frame   = 0;

                if (parser.type == FRAME_TYPE_SENSOR && parser.len == 1u) {
                    us_priority = parser.payload[0];
                    if (++debug_ctr >= 10) {
                        debug_ctr = 0;
                        debug_print_rx(us_priority);
                    }
                }

                if (in_safe_mode) {
                    in_safe_mode = 0;
                    RGB_BLUE_OFF();
                }

                if (us_priority != 0u)
                    RGB_RED_ON();
                else
                    RGB_RED_OFF();

                RGB_GREEN_ON();
            }
            /* PARSE_BAD_CRC: silently discard */
        }

        RGB_GREEN_OFF();

        /* ---- 2. Timeout fail-safe: 500 ms without valid frame ---- */
        if (!first_frame && !in_safe_mode &&
            (ms_ticks - last_valid_rx) >= 500u) {
            in_safe_mode = 1;
            RGB_BLUE_ON();
            RGB_RED_OFF();
            PRINTF("[CONTROL] SAFE MODE: no valid frame for 500ms\r\n");
        }
    }
}
