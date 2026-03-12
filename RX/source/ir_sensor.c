#include "ir_sensor.h"

/*
 * ir_sensor_init — configure all 6 IR GPIO pins as inputs with pull-up.
 * Call once from sensors_init_all() before entering the main loop.
 */
void ir_sensor_init(void)
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
void ir_sensor_read(uint8_t obs[IR_COUNT])
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
