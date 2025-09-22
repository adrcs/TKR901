/*---------------------------------------------------------------------------
	Project:	      MB1501 Programmer for PIC/Pi Zero

	Module:		      Frequency table entries

	File Name:	      frqtbl.c

	Author:		      MartinA

	Revision:	      1.00

	Description:      Frequency table entries

					This program is free software: you can redistribute it and/or modify
					it under the terms of the GNU General Public License as published by
					the Free Software Foundation, either version 2 of the License, or
					(at your option) any later version, provided this copyright notice
					is included.

					Copyright (c) Alberta Digital Radio Communications Society
					All rights reserved.


	Revision History:

---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>

#include "mb1501.h"

DEBUG_PINS testPins[N_TEST_MODES] = {
	{ LOG_TX_CLK,  LED_ON, LED_OFF },
	{ LOG_TX_DATA, LED_ON, LED_OFF },
	{ LOG_TX_LE, LED_ON, LED_OFF },
	{ LOG_RX_CLK,  LED_OFF, LED_ON },
	{ LOG_RX_DATA, LED_OFF, LED_ON },
	{ LOG_RX_LE, LED_OFF, LED_ON }
};

char *pindefs[] = {
	"TX_CLK",
	"TX_DATA",
	"TX_LE",
	"RX_CLK",	
	"RX_DATA",
	"RX_LE"
};

// tx synthesizer logical pin definitions
PROG_PINS txSynthLogPins = {
	.clkPin = LOG_TX_CLK,
	.dataPin = LOG_TX_DATA,
	.lePin = LOG_TX_LE
};

// rx synthesizer logical pin definitions
PROG_PINS rxSynthLogPins = {
	.clkPin = LOG_RX_CLK,
	.dataPin = LOG_RX_DATA,
	.lePin = LOG_RX_LE
};

void doPll(MB1501_ENTRY *entry, BOOL doTx, BOOL doRx);
void serialOut(PROG_PINS *logPins, uint32_t refDiv, uint32_t progDiv);
void doSerial(PROG_PINS *pins, uint32_t value, uint8_t cBit, uint8_t nBits);
void doTestMode(uint8_t logPin, uint8_t txLED, uint8_t rxLED);
void doLEDTest(void);

void doTableEntry(uint8_t tableValue)
{
	
#if __PI_ZERO
	fprintf(stderr, "Table entry %d\n", tableValue);
#endif		

	BOOL doTx=FALSE, doRx=FALSE;
	MB1501_ENTRY *entry = &TableEntries[tableValue];

	/*
	 * get the table entries and determine what to do...
	*/
	// amateur frequencies
	if ((tableValue >= AMATEUR_FREQUENCIES) && (tableValue <= AMATEUR_FREQUENCIES+N_AMATEUR-1)) {
#if __PI_ZERO
	fprintf(stderr, "Tuning to Amateur band\n");
#endif			
		doTx = TRUE, doRx = TRUE;
		doPll(entry, doTx, doRx);
		return;
	}

	// commercial frequencies
	if ((tableValue >= COMMERCIAL_FREQUENCIES) && (tableValue <= COMMERCIAL_FREQUENCIES+N_COMMERCIAL_FREQ - 1)) {
#if __PI_ZERO
	fprintf(stderr, "Tuning to Commercial band\n");
#endif	
		doTx = TRUE, doRx = TRUE;
		doPll(entry, doTx, doRx);
		return;
	}

	/*
	 * tune up modes
	 */

    // transmitter tuning
	if ((tableValue >= TX_TUNING) && (tableValue <= TX_TUNING+N_TX_TUNING-1)) {
#if __PI_ZERO
	fprintf(stderr, "Tx tuning mode\n");
#endif	
		doTx = TRUE, doRx = FALSE;
		doPll(entry, doTx, doRx);
		return;
	}

	// reciever tuning
	if ((tableValue >= RX_TUNING) && (tableValue <= RX_TUNING+N_RX_TUNING - 1)) {
		doTx = FALSE, doRx = TRUE;
#if __PI_ZERO
	fprintf(stderr, "Rx tuning mode\n");
#endif		
		doPll(entry, doTx, doRx);
		return;
	}

	/*
	 *	Debug modes
	 */
	if ((tableValue >= TEST_MODE_START) && (tableValue <= TEST_MODE_START+N_TEST_MODES - 1)) {
		DEBUG_PINS testPin = testPins[tableValue - TEST_MODE_START];
		doTestMode(testPin.logPin, testPin.txledMode, testPin.rxledMode);
		return;
	}

	if (tableValue == N_TABLE_ENTRIES - 1)
		doLEDTest();
	
#if __PI_ZERO
	fprintf(stderr, "Unused test mode selected\n");
#endif		
}

/*
 * Program the synthesizer with the table values
 */
void doPll(MB1501_ENTRY *entry, BOOL doTx, BOOL doRx)
{

	if (doRx) {
		printf("Rx Prog, %x, %x\n", entry->rxValue.refDiv, entry->rxValue.progCtr);
		setLED(RX_LED, LED_BLINK);
		serialOut(&rxSynthLogPins, entry->rxValue.refDiv, entry->rxValue.progCtr);
	}
	else {
		setLED(RX_LED, LED_OFF);
	}

	if (doTx) {
		printf("Tx Prog, %x, %x\n", entry->txValue.refDiv, entry->txValue.progCtr);
		setLED(TX_LED, LED_BLINK);
		serialOut(&txSynthLogPins, entry->txValue.refDiv, entry->txValue.progCtr);
	}
	else {
		setLED(TX_LED, LED_OFF);
	}
}

/*
 * do the actual serial operation
 * with a nod to Neal Reasoner, KB5ERY, SK
 * for the original code
 */
void serialOut(PROG_PINS *pins, uint32_t refDiv, uint32_t progDiv)
{
	setLogGpio(pins->clkPin, FALSE);
	setLogGpio(pins->dataPin, FALSE);
	setLogGpio(pins->lePin, FALSE);

	setLogGpioDir(pins->clkPin, LOG_PIN_OUTPUT);
	setLogGpioDir(pins->dataPin, LOG_PIN_OUTPUT);
	setLogGpioDir(pins->lePin, LOG_PIN_OUTPUT);

	doSerial(pins, refDiv, C_BIT, N_REF_BITS);

	doSerial(pins, progDiv, 0, N_DATA_BITS);
}

/*
 * serial communication to the MB1501
 */
void doSerial(PROG_PINS *pins, uint32_t value, uint8_t cBit, uint8_t nBits)
{

	// set the reference divider first
	uint32_t refSerOut = value << 1;
	refSerOut |= cBit;

#ifdef _DEBUG
	fprintf(stderr, "Serial out: %X: %X %d %d\n", refSerOut, value, cBit, nBits);
#endif

	uint32_t bitmask = 1 << (nBits - 1);
	for (int i = 0; i < nBits; i++) {
		uint8_t serBit = (refSerOut & bitmask) ? TRUE : FALSE;
		setLogGpio(pins->dataPin, serBit);

		delay();
		setLogGpio(pins->clkPin, TRUE);
		delay();
		setLogGpio(pins->clkPin, FALSE);

		bitmask >>= 1;
#ifdef _DEBUG
	fprintf(stderr, "%d", serBit);
#endif		
	}
	delay();
	setLogGpio(pins->lePin, TRUE);
	delay();
	setLogGpio(pins->lePin, FALSE);
	delay();
	
#ifdef _DEBUG
	fprintf(stderr, "\n");
#endif	

	// set the pins back
	setLogGpio(pins->dataPin, FALSE);
	setLogGpio(pins->clkPin, FALSE);
	setLogGpio(pins->lePin, FALSE);
	delay();
}

// send a square wave out on a pin
void doTestMode(uint8_t logPin, uint8_t txLED, uint8_t rxLED)
{
	setLED(TX_LED, txLED);
	setLED(RX_LED, rxLED);
	
#if __PI_ZERO
	fprintf(stderr, "Test mode: %s\n", pindefs[logPin]);
#endif	

	while (1) {
		delay();
		setLogGpio(logPin, TRUE);
		delay();
		setLogGpio(logPin, FALSE);
	}
}

// do the LED test: flash alternately
// (if equipped)
void doLEDTest(void)
{
	setLED(TX_LED, LED_ON);
	setLED(RX_LED, LED_OFF);
	
#if __PI_ZERO
	fprintf(stderr, "LED Test\n");
#endif	

	while (1) {
		for (int i = 0; i < LED_DELAY; i++)
			delay();
#if __PI_ZERO
	fprintf(stderr, "Tx OFF, Rx ON\n");
#endif			
		setLED(TX_LED, LED_OFF);
		setLED(RX_LED, LED_ON);

		for (int i = 0; i < LED_DELAY; i++)
			delay();
#if __PI_ZERO
	fprintf(stderr, "Tx ON, Rx OFF\n");
#endif			
		setLED(TX_LED, LED_ON);
		setLED(RX_LED, LED_OFF);
	}
}
