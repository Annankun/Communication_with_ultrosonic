#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include <stdint.h>
#include "MKL25Z4.h"

/* Requested remap:
 * TRIG: PTC3, PTC4, PTC5 (output)
 * ECHO: PTD0, PTD4, PTD5 (input + interrupt on PORTD)
 */

/* S1 LEFT: TRIG PTC3, ECHO PTD0 */
#define US1_TRIG_PORT   PORTC
#define US1_TRIG_GPIO   GPIOC
#define US1_TRIG_PIN    3u
#define US1_TRIG_MASK   (1u << US1_TRIG_PIN)

#define US1_ECHO_PORT   PORTD
#define US1_ECHO_GPIO   GPIOD
#define US1_ECHO_PIN    0u
#define US1_ECHO_MASK   (1u << US1_ECHO_PIN)

/* S2 RIGHT: TRIG PTC4, ECHO PTD4 */
#define US2_TRIG_PORT   PORTC
#define US2_TRIG_GPIO   GPIOC
#define US2_TRIG_PIN    4u
#define US2_TRIG_MASK   (1u << US2_TRIG_PIN)

#define US2_ECHO_PORT   PORTD
#define US2_ECHO_GPIO   GPIOD
#define US2_ECHO_PIN    4u
#define US2_ECHO_MASK   (1u << US2_ECHO_PIN)

/* S3 BACK: TRIG PTC5, ECHO PTD5 */
#define US3_TRIG_PORT   PORTC
#define US3_TRIG_GPIO   GPIOC
#define US3_TRIG_PIN    5u
#define US3_TRIG_MASK   (1u << US3_TRIG_PIN)

#define US3_ECHO_PORT   PORTD
#define US3_ECHO_GPIO   GPIOD
#define US3_ECHO_PIN    5u
#define US3_ECHO_MASK   (1u << US3_ECHO_PIN)

void Ultrasonic_InitAll(void);

uint32_t Ultrasonic_MeasureCm_Left(void);
uint32_t Ultrasonic_MeasureCm_Right(void);
uint32_t Ultrasonic_MeasureCm_Back(void);

#endif
