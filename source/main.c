/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "board.h"
#include "fsl_uart.h"
#include "LCD_4bit.h"
#include "LEDs.h"
#include "delay.h"
#include "pin_mux.h"
#include "timers.h"
#include "clock_config.h"
#include "GPIO_defs.h"
#include "GPS_helper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* UART instance and clock */
#define MAIN_UART UART2
#define MAIN_UART_CLKSRC BUS_CLK
#define MAIN_UART_CLK_FREQ CLOCK_GetFreq(BUS_CLK)
#define MAIN_UART_IRQn UART2_IRQn
#define UART_GPS_IRQn UART1_IRQn
#define GPS_BAUDRATE 9600

/*! @brief Ring buffer size (Unit: Byte). */
#define MAIN_RING_BUFFER_SIZE 512
#define RX_BUFFER_SIZE 512
#define MAX_SENTENCE 128
#define MAX_PARSE 256
#define STOP "STOP"
const char ledTest[] = "1";
const char ledTest2[] = "0";
volatile bool stop = 0;
volatile bool resume = 0;

/* Ring buffer to save received data. */

uint8_t mainRingBuffer[MAIN_RING_BUFFER_SIZE];
volatile uint16_t rxIndex = 0; //Write ISR
volatile uint16_t readIndex = 0; // Main loop

uint8_t mainRingBuffer_gps[MAIN_RING_BUFFER_SIZE];
volatile uint16_t rxIndex_gps = 0; //Write ISR
volatile uint16_t readIndex_gps = 0; // Main loop

/*LCD Print Buffers*/
char last_parsed_sentence[MAX_PARSE];
volatile bool newSentenceReady_gps = 0;

/*GPS Status*/
volatile char status = 'V';
volatile bool settingSpeed = 0;
volatile float speedMs = 0;
double angleCart = 0;
/*Print Check*/
volatile uint8_t printReady = 0;
volatile uint8_t lineDetectReady = 0;
char update[128];

volatile char cmdBuffer[10];
volatile int cmdIndex = 0;
volatile bool sent = 0;
char lastParsedSentence[MAX_PARSE];
volatile uint8_t newSentenceReady = 0;

const char onlyRMC[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";

/*******************************************************************************
 * Code
 ******************************************************************************/
void parse_RMC(char *sentence) {
	if (strncmp(sentence, "$GPRMC", 6) != 0) {
		return;
	}

	char *token;
	int field = 0;
	double latRaw = 0;
	double longRaw = 0;
	char ns = 'N', ew = 'E';
	double speedKnots = 0;
	status = 'V';
	char buf[128];
	strncpy(buf, sentence, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';
	token = strtok(buf, ",");
	while (token != NULL) {
		switch (field) {
		case 2:
			status = token[0];
			break;
		case 3:
			latRaw = atof(token);

			break;
		case 4:
			ns = token[0];
			break;
		case 5:
			longRaw = atof(token);

			break;
		case 6:
			ew = token[0];
			break;
		case 7:
			speedKnots = atof(token);
			break;
		case 8:
			angleCart = atof(token);
			break;
		}
		field++;
		token = strtok(NULL, ",");

	}

	double latDeg = ((int) (latRaw / 100))
			+ (latRaw - ((int) (latRaw / 100)) * 100) / 60.0;
	double lonDeg = ((int) (longRaw / 100))
			+ (longRaw - ((int) (longRaw / 100)) * 100) / 60.0;
	if (ns == 'S')
		latDeg = -latDeg;
	if (ew == 'W')
		lonDeg = -lonDeg;
	speedMs = speedKnots * 0.5144;
	double latMeters = 0;
	double lonMeters = 0;
	double currDist = 0;
	int closestBound = 0;
	double angle = 0;
	calcClosestBoundary(latDeg, lonDeg, &currDist, &closestBound, &angle);

	if (status == 'A') {
		snprintf(last_parsed_sentence, sizeof(last_parsed_sentence),
				"Status: %c | Lat: %2.4f | Lon: %2.4f | Speed: %2.2f |", status,
				latDeg, lonDeg, speedMs);
		//snprintf(lastParsedSentence, lastParsedSentenceSize, "%c", sentence);
	} else {
		snprintf(last_parsed_sentence, sizeof(last_parsed_sentence),
				"GPS Signal Invalid (Status: %c)", status);
	}
	newSentenceReady_gps = 1;

}
void parse_RX_NMEA(char c) {
	static char line[MAX_PARSE];
	static uint8_t idx = 0;
	if (c == '$') {
		idx = 0;
	}
	if (idx < MAX_PARSE - 1) {
		line[idx++] = c;
	} else {
		idx = 0;
		return;
	}
	if (c == '\n') {
		line[idx - 1] = '\0';
		parse_RMC(line);
		idx = 0;
	}
}
void Data_Output(char *string) {

	int i = 0;
	int j = 0;
	Set_Cursor(0, 0);

	while (*string && i < 16 && j < 2) {
		Delay(10);
		if (j == 0 && i >= 7 && (*string == ' ')) {
			i = 0;
			j = 1;
			Set_Cursor(i, j);

		}
		if (j == 1 && i >= 7 && (*string == ' ')) {
			i = 0;
			j = 0;
			Set_Cursor(i, j);
			Clear_LCD();
		}
		lcd_putchar(*string++);
		i++;

		if (i == 16) {
			j++;
			i = 0;
			if (j == 2 && (*string != '\0')) {
				j = 0;
				Clear_LCD();
			}
		}

		Set_Cursor(i, j);

	}
}

/*!
 * @brief Main function
 */
int main(void) {
	Init_RGB_LEDs();
	Control_RGB_LEDs(1, 0, 0);

	uart_config_t config;

	BOARD_InitPins();
	BOARD_BootClockRUN();
	Control_RGB_LEDs(0, 0, 1);
	//Init_RGB_LEDs();
	Init_PIT((5000 * 1000));
	Init_LCD();
	Clear_LCD();

	Set_Cursor(0, 0);

	/*
	 * config.baudRate_Bps = 9600;
	 * config.parityMode = kUART_ParityDisabled;
	 * config.stopBitCount = kUART_OneStopBit;
	 * config.txFifoWatermark = 0;
	 * config.rxFifoWatermark = 1;
	 * config.enableTx = false;
	 * config.enableRx = false;
	 */
	UART_GetDefaultConfig(&config);
	Control_RGB_LEDs(0, 1, 0);
	config.baudRate_Bps = GPS_BAUDRATE;
	config.enableTx = true;
	config.enableRx = true;
	Control_RGB_LEDs(0, 0, 0);
//	UART_Init(UART1, &config, MAIN_UART_CLK_FREQ);
//	UART_WriteBlocking(UART1, (uint8_t*) onlyRMC, sizeof(onlyRMC) - 1);
	UART_Init(UART2, &config, MAIN_UART_CLK_FREQ);

//	/* Enable RX interrupt. */
//	UART_EnableInterrupts(UART1,
//			kUART_RxDataRegFullInterruptEnable
//					| kUART_RxOverrunInterruptEnable);
//	EnableIRQ(UART1_IRQn);

	/* Enable RX interrupt. */
	UART_EnableInterrupts(UART2,
			kUART_RxDataRegFullInterruptEnable
					| kUART_RxOverrunInterruptEnable);
	EnableIRQ(UART2_IRQn);
	Start_PIT();
	Data_Output("Hello");
	while (1) {
//		while (stop) {
//			if (readIndex != rxIndex) {
//				char data = mainRingBuffer[readIndex];
//				readIndex = (readIndex + 1) % MAIN_RING_BUFFER_SIZE;
//				cmdBuffer[cmdIndex++] = data;
//
//				if (data == '1') {
//					Control_RGB_LEDs(0, 1, 0);
//				} else if (data == '0') {
//					Control_RGB_LEDs(1, 1, 0);
//				}
//
//			}
//
//			//Clear_LCD();
//		}
//		if (readIndex_gps != rxIndex_gps) {
//			uint8_t ch = mainRingBuffer_gps[readIndex_gps];
//			readIndex_gps = (readIndex_gps + 1) % MAIN_RING_BUFFER_SIZE;
//			parse_RX_NMEA(ch);
//
//			if (lineDetectReady) {
//				Clear_LCD();
//				Data_Output(update);
//				lineDetectReady = 0;
//
//			} else if (printReady) {
//				Clear_LCD();
//				Data_Output(last_parsed_sentence);
//				printReady = 0;
//			}
//
//		}
		if (readIndex != rxIndex) {
			char ch = mainRingBuffer[readIndex];
			readIndex = (readIndex + 1) % MAIN_RING_BUFFER_SIZE;
			//cmdBuffer[cmdIndex++] = ch;
			static char line[64];
			static uint8_t idx = 0;
			//Control_RGB_LEDs(1,0,1);
			if (idx < sizeof(line)-1 && ch != '\0') {
				line[idx++] = ch;
			} else {
				idx = 0;

			}
			if (ch == '\n') {
				Control_RGB_LEDs(0,1,1);
				line[idx-1] = '\0';
				Clear_LCD();
				Data_Output(line);
				idx = 0;
				Control_RGB_LEDs(0,0,0);
			}

		}
		//UART_WriteBlocking(UART2, (uint8_t*) ledTest2, sizeof(ledTest2) - 1);

	}
	//Control_RGB_LEDs(1, 0, 0);
	//Stop_PIT();
}

void PIT_IRQHandler() {
	//unsigned short out_data = 0;

	//clear pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);

	// check to see which channel triggered interrupt
	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) { //Check against bit 31 which corresponds to timer interrupt flag
		// clear status flag for timer channel 0
		PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK; //Write 1 to clear the flag
		if (newSentenceReady_gps) {
			printReady = 1;
			newSentenceReady_gps = 0;
		}
//

	} else if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 1
		PIT->CHANNEL[1].TFLG &= PIT_TFLG_TIF_MASK;
	}
}
//void UART1_IRQHandler(void) {
//
//	/* If new data arrived. */
//	if (UART_GetStatusFlags(UART1) & kUART_RxDataRegFullFlag) {
//		uint8_t data = UART_ReadByte(UART1);
//		uint16_t next = ((rxIndex_gps + 1) % MAIN_RING_BUFFER_SIZE);
//		/* If ring buffer is not full, add data to ring buffer. */
//		if (next != readIndex_gps) {
//			mainRingBuffer_gps[rxIndex_gps] = data;
//			rxIndex_gps = next;
//		}
//	}
//	if (UART_GetStatusFlags(UART1) & kUART_RxOverrunFlag) {
//
//		UART_ClearStatusFlags(UART1, kUART_RxOverrunFlag);
//
//	}
//}
void UART2_IRQHandler(void) {

	/* If new data arrived. */
	if (UART_GetStatusFlags(UART2) & kUART_RxDataRegFullFlag) {
		uint8_t data = UART_ReadByte(UART2);
		uint16_t next = ((rxIndex + 1) % MAIN_RING_BUFFER_SIZE);
		/* If ring buffer is not full, add data to ring buffer. */
		if (next != readIndex) {
			mainRingBuffer[rxIndex] = data;
			rxIndex = next;
		}
	}
	if (UART_GetStatusFlags(UART2) & kUART_RxOverrunFlag) {

		UART_ClearStatusFlags(UART2, kUART_RxOverrunFlag);

	}
}

