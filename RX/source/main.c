

#include "MKL25Z4.h"
#include "pin_config_rx.h"
#include "pin_config.h"
#include "uart.h"
#include "debug_uart.h"
#include "ringbuf.h"
#include "protocol.h"
#include "sensor_status.h"
#include "sensor_sample.h"

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


static void debug_putdec32(int32_t n)
{
    char tmp[11];
    int  i = 0;
    uint32_t uval;

    if (n < 0) {
        debug_putchar('-');
        uval = (uint32_t)(-(n + 1)) + 1u;
    } else {
        debug_putchar('+');
        uval = (uint32_t)n;
    }

    if (uval == 0) { debug_putchar('0'); return; }
    while (uval > 0) { tmp[i++] = '0' + (char)(uval % 10); uval /= 10; }
    while (i > 0) debug_putchar(tmp[--i]);
}

static void debug_print_rx(const snapshot_t *s)
{
    uint8_t i;

    PRINTF("[RX] IR=");
    for (i = 0; i < IR_COUNT; i++) debug_putchar(s->ir_obs[i] ? '1' : '0');
    PRINTF(" US=");
    for (i = 0; i < US_COUNT; i++) debug_putchar(s->us_obs[i] ? '1' : '0');
    PRINTF(" TOF="); debug_putchar(s->tof_obstacle ? '1' : '0');
    PRINTF(" GPS="); debug_putchar(s->gps_valid ? '1' : '0');
    PRINTF(" LAT="); debug_putdec32(s->lat_deg7);
    PRINTF(" LON="); debug_putdec32(s->lon_deg7);
    PRINTF("\r\n");
}

int main(void)
{
    snapshot_t snap;
    parser_t   parser;
    uint8_t    c;
    uint8_t    debug_ctr  = 0;

    uint32_t last_valid_rx = 0;
    uint8_t  first_frame   = 1;
    uint8_t  in_safe_mode  = 0;
    uint8_t  in_estop      = 0;

    /* Default snapshot: all clear */
    uint8_t i;
    for (i = 0; i < IR_COUNT; i++) snap.ir_obs[i] = 1;
    for (i = 0; i < US_COUNT; i++) snap.us_obs[i] = 1;
    snap.tof_obstacle = 1;
    snap.gps_valid    = 0;
    snap.lat_deg7     = 0;
    snap.lon_deg7     = 0;

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000u);

    pin_config_init();
    pin_config_rx_init();
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
        if (in_estop) {
            /* Flush ring and reset parser so we start clean on resume */
            while (uart2_getchar(&c))
                ;
            parser_init(&parser);
            RGB_GREEN_OFF();
        } else {
            while (uart2_getchar(&c)) {
                int result = parser_feed(&parser, c);

                if (result == PARSE_OK) {
                    last_valid_rx = ms_ticks;
                    first_frame   = 0;

                    if (parser.type == FRAME_TYPE_SENSOR &&
                        parser.len  == SNAPSHOT_PAYLOAD_BYTES) {
                        snapshot_unpack(&snap, parser.payload);
                        if (++debug_ctr >= 10) {
                            debug_ctr = 0;
                            debug_print_rx(&snap);
                        }
                    }

                    if (in_safe_mode) {
                        in_safe_mode = 0;
                        if (!in_estop) RGB_BLUE_OFF();
                    }

                    if (!in_estop) {
                        if (snapshot_any_obstacle(&snap))
                            RGB_RED_ON();
                        else
                            RGB_RED_OFF();
                    }

                    RGB_GREEN_ON();
                }
                /* PARSE_BAD_CRC: silently discard */
            }

            RGB_GREEN_OFF();
        }

        /* ---- 2. Timeout fail-safe: 500 ms without valid frame ---- */
        if (!first_frame && !in_safe_mode && !in_estop &&
            (ms_ticks - last_valid_rx) >= 500u) {
            in_safe_mode = 1;
            RGB_BLUE_ON();
            RGB_RED_OFF();
            PRINTF("[CONTROL] SAFE MODE: no valid frame for 500ms\r\n");
        }
    }
}
