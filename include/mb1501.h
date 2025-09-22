/*---------------------------------------------------------------------------
	Project:	      MB1501 Programmer for PIC/Pi Zero

	Module:		      MB1501 table definitions

	File Name:	      mb1501.h

	Date Created:	  Jan 9, 2025

	Author:			  MartinA

	Description:      <what it does>

					  Copyright © 2024-25, Alberta Digital Radio Communications Society,
					  All rights reserved

	NB:				  During compilation define __PI_ZERO for the Pi, __PIC for the PIC.
					  Leaving that out will generate code for the PC only

	Revision History:

---------------------------------------------------------------------------*/
#ifndef __M1501_H
#define __M1501_H

#include <stdint.h>

//define the platform we are on
#define	__PI_ZERO		1		// pi zero
#define	__PIC			0		// in a pic
#define __PC			0		// on a pc

// MB1501 fields
typedef struct mb1501_v {
	uint32_t refDiv;			// reference divider
	uint32_t progCtr;			// programmable counter
} MB1501_value;

typedef struct tblent_t {
	MB1501_value	txValue;	// tx values
	MB1501_value	rxValue;	// rx values
} MB1501_ENTRY;

// MB1501 definitions
#define	N_REF_BITS		16		// reference bits
#define	N_DATA_BITS		19		// number of data bits
#define	C_BIT			1		// c-bit, when 1 set ref, when 0 set divider

// LED definitions
#define	TX_LED			0		// Tx Led
#define	RX_LED			1		// Rx LED
#define	LED_DELAY		1600		// Blink delay

// table definitions
#define	AMATEUR_FREQUENCIES		0						// start of amateur frequencies
#define N_AMATEUR				80						// number of frequencies
#define	TX_TUNING				N_AMATEUR				// start of tx tuning frequencies
#define	N_TX_TUNING				33						// number of tx tuning frequencies
#define	RX_TUNING				TX_TUNING+N_TX_TUNING	// start of rx tuning
#define N_RX_TUNING				15						// number of rx
#define COMMERCIAL_FREQUENCIES	RX_TUNING+N_RX_TUNING	// start of commercial frequencies
#define	N_COMMERCIAL_FREQ		99						// number of commercial frequencies
#define	TEST_MODE_START			240						// test modes
#define	N_TEST_MODES			6						// number of test modes
#define N_TABLE_ENTRIES			256						// number of table entries

// typedefs
typedef enum 
{
	AMATEUR_MODE=0,				// amateur mode
	TX_TUNING_MODE,				// tx tuning
	RX_TUNING_MODE,				// rx tuning
	COMMERCIAL_MODE,			// commercial mode
	TEST_MODE					// test mode
} OpModes;

#define	FALSE		0
#define	TRUE		1
typedef	uint8_t		BOOL;

// logical pin defs
typedef enum logpins_e {
	LOG_TX_CLK=0,
	LOG_TX_DATA,
	LOG_TX_LE,
	LOG_RX_CLK,
	LOG_RX_DATA,
	LOG_RX_LE
} logPins;

// logical pin directions
typedef enum logpindir_e	{
	LOG_PIN_INPUT=0,
	LOG_PIN_OUTPUT
} logPinDir;

// led modes
typedef enum ledmodes_e	{
	LED_OFF,
	LED_BLINK,
	LED_ON
} ledModes;

// pin logical definitons
typedef struct debugPins_t {
	uint8_t	logPin;
	uint8_t txledMode;
	uint8_t rxledMode;
} DEBUG_PINS;

typedef struct prog_pins_t {
	uint8_t clkPin;				// clock pin
	uint8_t dataPin;			// data pin
	uint8_t lePin;				// LE pin
} PROG_PINS;

// externs
void doTableEntry(uint8_t entry);
void doPll(MB1501_ENTRY *entry, BOOL doTx, BOOL doRx);

// device dependent routines
void setLogGpio(uint8_t logPin, uint8_t value);
void setLogGpioDir(uint8_t logPin, uint8_t direction);
void delay(void);
void setLED(uint8_t led, uint8_t value);

extern MB1501_ENTRY TableEntries[];

#endif
