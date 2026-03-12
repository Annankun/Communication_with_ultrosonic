
#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "MKL25Z4.h"
#include "pin_config.h"     //SIM clock gates, PORT_PCR_MUX macros
#include "sensor_status.h"  //IR_COUNT = 6

static inline void ir_sensor_init(void)
{

    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;

    // PTE3, PTE2 — GPIO input
    PORTE->PCR[3] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTE->PCR[2] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    GPIOE->PDDR  &= ~((1u << 3) | (1u << 2));   /* set as input */

    //* PTB11, PTB10, PTB9, PTB8 — GPIO input
    PORTB->PCR[11] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[10] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[9]  = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTB->PCR[8]  = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    GPIOB->PDDR   &= ~((1u << 11) | (1u << 10) | (1u << 9) | (1u << 8));
}


static inline void ir_sensor_read(uint8_t obs[IR_COUNT])
{
    uint32_t pdir_e = GPIOE->PDIR;
    uint32_t pdir_b = GPIOB->PDIR;

    obs[0] = (uint8_t)((pdir_e >> 3)  & 1u);   //PTE3
    obs[1] = (uint8_t)((pdir_e >> 2)  & 1u);   //PTE2
    obs[2] = (uint8_t)((pdir_b >> 11) & 1u);   //PTB11
    obs[3] = (uint8_t)((pdir_b >> 10) & 1u);   //PTB10
    obs[4] = (uint8_t)((pdir_b >> 9)  & 1u);   //PTB9
    obs[5] = (uint8_t)((pdir_b >> 8)  & 1u);   //PTB8
}

#endif
