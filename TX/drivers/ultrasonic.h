#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include <stdint.h>
#include "MKL25Z4.h"

/* S3 BACK: TRIG PTD0, ECHO PTD2 */
#define US3_TRIG_PORT   PORTD
#define US3_TRIG_GPIO   GPIOD
#define US3_TRIG_PIN    0u
#define US3_TRIG_MASK   (1u << US3_TRIG_PIN)

#define US3_ECHO_PORT   PORTD
#define US3_ECHO_GPIO   GPIOD
#define US3_ECHO_PIN    2u
#define US3_ECHO_MASK   (1u << US3_ECHO_PIN)

/* S1 LEFT: TRIG PTD7, ECHO PTD16 */
#define US1_TRIG_PORT   PORTD
#define US1_TRIG_GPIO   GPIOD
#define US1_TRIG_PIN    7u
#define US1_TRIG_MASK   (1u << US1_TRIG_PIN)

#define US1_ECHO_PORT   PORTD
#define US1_ECHO_GPIO   GPIOD
#define US1_ECHO_PIN    16u
#define US1_ECHO_MASK   (1u << US1_ECHO_PIN)

/* S2 RIGHT: TRIG PTD4, ECHO PTD5 */
#define US2_TRIG_PORT   PORTD
#define US2_TRIG_GPIO   GPIOD
#define US2_TRIG_PIN    4u
#define US2_TRIG_MASK   (1u << US2_TRIG_PIN)

#define US2_ECHO_PORT   PORTD
#define US2_ECHO_GPIO   GPIOD
#define US2_ECHO_PIN    5u
#define US2_ECHO_MASK   (1u << US2_ECHO_PIN)

void Ultrasonic_InitAll(void);

uint32_t Ultrasonic_MeasureCm_Left(void);
uint32_t Ultrasonic_MeasureCm_Right(void);
uint32_t Ultrasonic_MeasureCm_Back(void);

#endif
