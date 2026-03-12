#include "timers.h"
#include "MKL25Z4.h"

// global variable counting # of 10 us ticks
// incremented inside the PIT interrupt handler
// volatile -> avoids compiler skipping over
volatile unsigned g_ticks10us = 0;

// configures PIT channel 0 to generate interrupt every 10 us
void Init_PIT0_10us(void)
{
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK; // enables clock
    PIT->MCR &= ~PIT_MCR_MDIS_MASK; // clears MDIS (enables module)
    PIT->MCR |= PIT_MCR_FRZ_MASK; // sets FRZ bit -> debugging

    // 24 MHz bus clock (24 cycles per us)
    // 10 us = 240 cycles -> LDVAL = 240 - 1 = 239
    // 0 -> triggers timeout event
    PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(239u);

    // disables chaining -> no other timer is triggered
    PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_CHN_MASK;

    // enables interrupts for PIT0
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;

    // middle priority
    NVIC_SetPriority(PIT_IRQn, 128);
    // clear old pending interrupts
    NVIC_ClearPendingIRQ(PIT_IRQn);
    // enable PIT interrupts @ NVIC level -> CPU jumps to PIT_IRQHandler()
    NVIC_EnableIRQ(PIT_IRQn);
}

void Start_PIT0(void)
{
	// sets timer enable bit -> PIT0 counts down repeatedly
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}

void Stop_PIT0(void)
{
	// clears timer enable bit -> PIT0 stops counting
    PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;
}

unsigned Timer_Micros(void)
{
    // converts 10us ticks to microseconds
    return g_ticks10us * 10u;
}

// PIT interrupt handler (ISR)
void PIT_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(PIT_IRQn); // clears pending state in NVIC

    // checks that PIT0 actually timed out
    // clears PIT0 interrupt flag -> stops retrigger of interrupt
    if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
        PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
        g_ticks10us++; // increments per interrupt
    }
}
