#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "MKL25Z4.h"
#include "pin_config.h"     //SIM clock gates, PORT_PCR_MUX macros
#include "sensor_status.h"  //IR_COUNT = 6

void ir_sensor_init(void);
void ir_sensor_read(uint8_t obs[IR_COUNT]);

#endif
