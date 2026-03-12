/*
 * ir_sensor.h — IR Obstacle Sensor Driver
 *
 * Wraps the 6 IR GPIO pins into a clean two-function driver interface:
 *
 *   ir_sensor_init()          — configure pins once at startup
 *   ir_sensor_read(obs[6])    — poll all 6 sensors, fill obs[] with 0/1
 *
 * ======================================================================
 * TEMPLATE NOTE FOR TEAMMATES
 * ======================================================================
 * Copy this file and adapt it to write your own driver.
 * Keep the same two-function pattern:
 *
 *   void xxx_sensor_init(void)           — hardware setup, called once
 *   void xxx_sensor_read(<out params>)   — read hardware, no side effects
 *
 * Rules:
 *   1. Do NOT write to g_sensor_status inside these functions.
 *      update_sensor_status() in sensor_board/main.c does that.
 *   2. Do NOT call delay_ms() or block inside _read().
 *      Polling must be fast and non-blocking.
 *   3. Keep _init() and _read() stateless where possible.
 *      Any required state should be static local to this file.
 *
 * Template files to create:
 *   sensor_board/ultrasonic_sensor.h    (obs[4], 0=obstacle 1=clear)
 *   sensor_board/tof_sensor.h           (obstacle, 0=obstacle 1=clear)
 *   sensor_board/gps_sensor.h           (valid, lat_deg7, lon_deg7)
 * ======================================================================
 *
 * Hardware mapping (FRDM-KL25Z):
 *   Index   Pin     Port/GPIO   Active-low (0 = obstacle, 1 = clear)
 *     0     PTE3    PORTE bit3
 *     1     PTE2    PORTE bit2
 *     2     PTB11   PORTB bit11
 *     3     PTB10   PORTB bit10
 *     4     PTB9    PORTB bit9
 *     5     PTB8    PORTB bit8
 */

#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "MKL25Z4.h"
#include "pin_config.h"     /* SIM clock gates, PORT_PCR_MUX macros */
#include "sensor_status.h"  /* IR_COUNT = 6 */

void ir_sensor_init(void);
void ir_sensor_read(uint8_t obs[IR_COUNT]);

#endif /* IR_SENSOR_H */
