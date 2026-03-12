#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

void Servo_Init(void);
void Servo1_SetAngle(uint8_t angle);
void Servo2_SetAngle(uint8_t angle);

#endif
