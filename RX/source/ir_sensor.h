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

/*
 * ir_sensor_init — configure all 6 IR GPIO pins as inputs with pull-up.
 * Call once from sensors_init_all() before entering the main loop.
 */
static inline void ir_sensor_init(void)
{
    /* Enable clocks for PORTB and PORTE (pin_config_init() covers this,
     * but guard here so the driver works even if called in isolation). */
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;

    /* PTE3, PTE2 — GPIO input, internal pull-up enabled */
    PORTE->PCR[3] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTE->PCR[2] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    GPIOE->PDDR  &= ~((1u << 3) | (1u << 2));   /* set as input */

    /* PTB11, PTB10, PTB9, PTB8 — GPIO input, internal pull-up enabled */
    PORTB->PCR[11] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[10] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[9]  = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[8]  = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    GPIOB->PDDR   &= ~((1u << 11) | (1u << 10) | (1u << 9) | (1u << 8));
}

/*
 * ir_sensor_read — read all 6 IR sensors in one atomic GPIO snapshot.
 *
 * obs[0..5]: 0 = obstacle detected, 1 = path clear  (active-low sensor)
 *
 * Reads each port's PDIR register once to guarantee a consistent sample
 * across all pins (no tearing between individual bit reads).
 */
static inline void ir_sensor_read(uint8_t obs[IR_COUNT])
{
    uint32_t pdir_e = GPIOE->PDIR;   /* snapshot PORTE */
    uint32_t pdir_b = GPIOB->PDIR;   /* snapshot PORTB */

    obs[0] = (uint8_t)((pdir_e >> 3)  & 1u);   /* PTE3  */
    obs[1] = (uint8_t)((pdir_e >> 2)  & 1u);   /* PTE2  */
    obs[2] = (uint8_t)((pdir_b >> 11) & 1u);   /* PTB11 */
    obs[3] = (uint8_t)((pdir_b >> 10) & 1u);   /* PTB10 */
    obs[4] = (uint8_t)((pdir_b >> 9)  & 1u);   /* PTB9  */
    obs[5] = (uint8_t)((pdir_b >> 8)  & 1u);   /* PTB8  */
}

#endif /* IR_SENSOR_H */
