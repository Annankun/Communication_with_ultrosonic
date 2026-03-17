

#include "MKL25Z4.h"
#include "ir_sensor.h"
#include "uart.h"
#include "debug_uart.h"
#include "ringbuf.h"
#include "protocol.h"
#include "sensor_status.h"
#include "sensor_sample.h"
#include "delay.h"
#include "ultrasonic.h"
#include "timers.h"
#include "lcd.h"


sensor_status_t g_sensor_status;

/* Globals required by uart.h (TX board discards incoming bytes) */
ringbuf_t         rx_ring;
volatile uint32_t rx_overflow_count;

void UART2_IRQHandler(void)
{
    uint8_t status = COMM_UART->S1;
    if (status & (UART_S1_RDRF_MASK | UART_S1_OR_MASK))
        (void)COMM_UART->D;   /* discard */
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

#define SIDE_THRESH_CM   10u
#define BACK_THRESH_CM   10u

static void tof_init(void) { /* TODO */ }

static void tof_read(uint8_t *obstacle)
{
    /* TODO: read distance; set *obstacle = (dist_mm < threshold) ? 0 : 1 */
    *obstacle = 1;
}

static void gps_init(void) { /* TODO */ }

static void gps_read(uint8_t *valid, int32_t *lat_deg7, int32_t *lon_deg7)
{
    /* TODO: parse latest NMEA sentence */
    *valid    = 0;
    *lat_deg7 = 0;
    *lon_deg7 = 0;
}

/* ====================================================================
 * sensors_init_all / update_sensor_status
 * ==================================================================== */

static void sensors_init_all(void)
{
    ir_sensor_init();
    Ultrasonic_InitAll();
   // Servo_Init();
    tof_init();
    gps_init();
}

static void update_sensor_status(void)
{
    uint32_t dist_s1;
    uint32_t dist_s2;
    uint32_t dist_s3;

    ir_sensor_read(g_sensor_status.ir_obs);

    dist_s1 = Ultrasonic_MeasureCm_Left();
    dist_s2 = Ultrasonic_MeasureCm_Right();
    dist_s3 = Ultrasonic_MeasureCm_Back();

    if (dist_s3 > 0u && dist_s3 < BACK_THRESH_CM) {
        g_sensor_status.us_priority = 3u;
    } else if ((dist_s1 > 0u && dist_s1 < SIDE_THRESH_CM) &&
               (dist_s2 > 0u && dist_s2 < SIDE_THRESH_CM)) {
        g_sensor_status.us_priority = (dist_s1 < dist_s2) ? 1u : 2u;
    } else if (dist_s1 > 0u && dist_s1 < SIDE_THRESH_CM) {
        g_sensor_status.us_priority = 1u;
    } else if (dist_s2 > 0u && dist_s2 < SIDE_THRESH_CM) {
        g_sensor_status.us_priority = 2u;
    } else {
        g_sensor_status.us_priority = 0u;
    }

    tof_read(&g_sensor_status.tof_obstacle);
    gps_read(&g_sensor_status.gps_valid,
             &g_sensor_status.lat_deg7,
             &g_sensor_status.lon_deg7);
}

/* ====================================================================
 * debug_print_tx
 * Format: [TX] IR=110010 US_PRI=2 TOF=1 GPS=1 LAT=+374230000 LON=-1220840000
 * ==================================================================== */

static void debug_putdec32(int32_t n)
{
    char tmp[11];
    int  i = 0;
    uint32_t uval;

    if (n < 0) {
        debug_putchar('-');
        /* avoid UB on INT32_MIN */
        uval = (uint32_t)(-(n + 1)) + 1u;
    } else {
        debug_putchar('+');
        uval = (uint32_t)n;
    }

    if (uval == 0) { debug_putchar('0'); return; }
    while (uval > 0) { tmp[i++] = '0' + (char)(uval % 10); uval /= 10; }
    while (i > 0) debug_putchar(tmp[--i]);
}

static void debug_print_tx(const snapshot_t *s)
{
    uint8_t i;

    PRINTF("[TX] IR=");
    for (i = 0; i < IR_COUNT; i++) debug_putchar(s->ir_obs[i] ? '1' : '0');
    PRINTF(" US_PRI=");
    debug_putchar((char)('0' + s->us_priority));
    PRINTF(" TOF="); debug_putchar(s->tof_obstacle ? '1' : '0');
    PRINTF(" GPS="); debug_putchar(s->gps_valid ? '1' : '0');
    PRINTF(" LAT="); debug_putdec32(s->lat_deg7);
    PRINTF(" LON="); debug_putdec32(s->lon_deg7);
    PRINTF("\r\n");
}


static const char *us_priority_text(uint8_t p)
{
    switch (p) {
    case 1u: return "US: LEFT!";
    case 2u: return "US: RIGHT!";
    case 3u: return "US: BACK!";
    default: return "US: CLEAR";
    }
}

static void lcd_update_us_if_changed(uint8_t us_priority)
{
    static uint8_t last_us_priority = 0xFFu;

    if (us_priority == last_us_priority) {
        return;
    }

    last_us_priority = us_priority;
    Clear_LCD();
    Set_Cursor(0u, 0u);
    Print_LCD((char *)us_priority_text(us_priority));
}

/* ====================================================================
 * main
 * ==================================================================== */

int main(void)
{
    uint8_t    seq        = 0;
    uint8_t    debug_ctr  = 0;
    snapshot_t snap;
    uint8_t    payload[SNAPSHOT_PAYLOAD_BYTES];
    uint8_t    frame_buf[FRAME_HEADER_SIZE + SNAPSHOT_PAYLOAD_BYTES + FRAME_CRC_SIZE];

    /* Servo scan state */
    static uint8_t s_angle = 0;
    static uint8_t s_dir   = 1;   /* 1=increasing, 0=decreasing */

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000u);

    pin_config_init();
    sensors_init_all();
    uart2_init();
    debug_uart_init();

    RGB_ALL_OFF();

    /* Startup blink: 3x green */
    for (int b = 0; b < 3; b++) {
        RGB_GREEN_ON();  delay_ms(150);
        RGB_GREEN_OFF(); delay_ms(150);
    }

    Init_LCD();
    lcd_update_us_if_changed(0u);

    PRINTF("[SENSOR] Sensor board ready. Polling all sensors at 10 Hz.\r\n");

    while (1) {
        /* 1. Read all sensors → g_sensor_status */
        update_sensor_status();

        /* 2. Copy g_sensor_status into snapshot */
        snapshot_sample(&snap);

        /* 2.1 Update LCD only when US priority changes */
        lcd_update_us_if_changed(snap.us_priority);

        /* 3. Red LED = any obstacle */
        if (snapshot_any_obstacle(&snap))
            RGB_RED_ON();
        else
            RGB_RED_OFF();

        /* 4. Serialize → frame → transmit */
        snapshot_pack(&snap, payload);
        uint8_t frame_len = frame_pack(frame_buf, FRAME_TYPE_SENSOR, seq,
                                       payload, SNAPSHOT_PAYLOAD_BYTES);
        uint8_t i;
        for (i = 0; i < frame_len; i++)
            uart2_putchar(frame_buf[i]);

        seq++;

        /* 5. Green flash: frame sent */
        RGB_GREEN_ON();  delay_ms(10);
        RGB_GREEN_OFF();

        /* 6. Debug print every 10 frames */
        if (++debug_ctr >= 10) {
            debug_ctr = 0;
            debug_print_tx(&snap);
        }

//        /* 7. Servo scan */
//        Servo1_SetAngle(s_angle);
//        Servo2_SetAngle(180u - s_angle);
//        if (s_dir) {
//            if (s_angle < 150u) s_angle += 15u;
//            else s_dir = 0;
//        } else {
//            if (s_angle > 15u) s_angle -= 15u;
//            else s_dir = 1;
//        }

        /* 8. Wait for next cycle */
        delay_ms(75);
    }
}
