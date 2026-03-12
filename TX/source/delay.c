#include "delay.h"

void Delay(uint32_t dly)
{
    volatile uint32_t t;
    for (t = dly * 10000; t > 0; t--) {
        ;
    }
}
