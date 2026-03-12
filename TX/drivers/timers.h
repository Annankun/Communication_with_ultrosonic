#ifndef TIMERS_H
#define TIMERS_H

#include "MKL25Z4.h"

void Init_PIT0_10us(void);
void Start_PIT0(void);
void Stop_PIT0(void);

unsigned Timer_Micros(void);

#endif
