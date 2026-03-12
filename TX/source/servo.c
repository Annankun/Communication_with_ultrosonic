#include "servo.h"
#include "MKL25Z4.h"

/*
 * Two MG90S Servos
 * Servo 1: PTA12 -> TPM1_CH0
 * Servo 2: PTA13 -> TPM1_CH1
 * 50 Hz PWM
 */

void Servo_Init(void)
{
    // Enable clocks
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;

    // PTA12 -> TPM1_CH0, PTA13 -> TPM1_CH1
    PORTA->PCR[12] = PORT_PCR_MUX(3);
    PORTA->PCR[13] = PORT_PCR_MUX(3);

    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

    /*
     * Prescaler = /16
     * TPM clock = 12 MHz / 16 = 750 kHz
     * 50 Hz PWM (20 ms)
     */
    TPM1->SC = 0;
    TPM1->MOD = 15000;

    // Edge-aligned PWM, high-true pulses
    TPM1->CONTROLS[0].CnSC =
        TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
    TPM1->CONTROLS[1].CnSC =
        TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;

    /* Start TPM */
    TPM1->SC = TPM_SC_PS(4) | TPM_SC_CMOD(1);
}

static uint32_t angle_to_cnv(uint8_t angle)
{
    if (angle > 180) angle = 180;

    // 0° ≈ 0.6 ms (450), 180° ≈ 2.4 ms (1800)
    return 450 + ((uint32_t)angle * 1350) / 180;
}

void Servo1_SetAngle(uint8_t angle)
{
    TPM1->CONTROLS[0].CnV = angle_to_cnv(angle);
}

void Servo2_SetAngle(uint8_t angle)
{
    TPM1->CONTROLS[1].CnV = angle_to_cnv(angle);
}
