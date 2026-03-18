#include "timers.h"
#include "MKL25Z4.h"

#include "LEDs.h"

volatile unsigned PIT_interrupt_counter = 0;


void Init_PIT(unsigned period_us) {
	// Enable clock to PIT module
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	
	// Enable module, freeze timers in debug mode
	PIT->MCR &= ~PIT_MCR_MDIS_MASK; //Enables clock for standard timers for PIT
	PIT->MCR |= PIT_MCR_FRZ_MASK; //
	
	// Initialize PIT0 to count down from argument 
	PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(period_us*24); // 24 MHz clock frequency

	// No chaining
	PIT->CHANNEL[0].TCTRL &= PIT_TCTRL_CHN_MASK; //Timer 0 cannot be chained
	

	// Generate interrupts
	//Bit 30 sets timer interrupt enable
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK; //Typically should clear flag in case that enable interrupts

	/* Enable Interrupts */
	NVIC_SetPriority(PIT_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(PIT_IRQn); 
	NVIC_EnableIRQ(PIT_IRQn);	
}


void Start_PIT(void) {
// Enable counter
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; //Enable the timer through the timer enable bit (31)
}

void Stop_PIT(void) {
// Enable counter
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //Disable the timer through the timer enable bit (30)
}




// *******************************ARM University Program Copyright � ARM Ltd 2013*************************************   
