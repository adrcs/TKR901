/*---------------------------------------------------------------------------
	Project:	      MB1501 Programmer for PIC/Pi Zero

	Module:		      mainline for Pi 

	File Name:	      main.c

	Author:		      MartinA, VE6VH

	Revision:	      1.00

	Description:      Provide a mainline for the Pi Zero mode

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
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "mb1501.h"

#if __PI_ZERO
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

// GPIO macros for Raspberry Pi
#include <fcntl.h>
#include <sys/mman.h>

// Pi physical pin assignments
/* P1 connector pinout - 40 pin ******************************************
*		P1-1	3.3v			  J2/1          P1-2	5.0v
*		P1-3	GPIO 2 SDA1		                P1-4	5.0v
*		P1-5	GPIO 3 SCL1		                P1-6	Ground
*		P1-7	GPIO 4			                P1-8	GPIO14	TXD0
*		P1-9	Ground			                P1-10	GPIO15	RXD0
*		P1-11	RX_CLK			  J3/1          P1-12	GPIO18
*		P1-13	RX_DATA			  J3/2          P1-14	Ground           J4/1
*		P1-15	RX_LE			  J3/3          P1-16	GPIO 23          J4/2
*		P1-17	3.3v			  J3/4          P1-18	RX_LD            J4/3
*		P1-19	GPIO 10 SPIMOSI	                P1-20	Ground
*		P1-21	GPIO 9 SPI_MISO	                P1-22	GPIO25
*		P1-23	GPIO 11 SPISCLK	                P1-24	GPIO8	SPI_CE0_n
*		P1-25	GND		          J1/1          P1-26	GPIO7   SPI_CE1_n
*       P1-27   ID SD  (N/C)      J1/2          P1-28   ID SC
*       P1-29   TX_CLK            J1/3          P1-30   Ground
*       P1-31   TX_DATA           J1/4          P1-32   GPIO 12
*       P1 33   TX_LE             J1/5          P1-34   Ground
*       P1-35   TX_LD             J1/6          P1-36   GPIO 16
*       P1-37   GPIO 26                         P1-38   GPIO 20 PCM DIN
*       P1-39 	Ground                          P1-40   GPIO 21 PCM DOUT
*
*************************************************************************/
// global for GPIO access
volatile unsigned *gpio;

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define BLOCK_SIZE (4*1024)

#define PHY_TX_CLK      5		// Connector P1 - pin 29
#define PHY_TX_DATA     6		// Connector P1 - pin 31
#define PHY_TX_ENA      13		// Connector P1 - pin 33

#define PHY_RX_CLK      17		// Connector P1 - pin 11
#define PHY_RX_DATA     27		// Connector P1 - pin 13
#define PHY_RX_ENA      22		// Connector P1 - pin 15

#define PHY_TX_LD       19      // pin 35: Tx lock detect
#define PHY_RX_LD       24      // pin 18: Lock detect

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define PI_INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define PI_OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define PI_SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define PI_GPIO_SET(g) (g<32 ? (*(gpio+7)=(1<<g)) : (*(gpio+8)=(1<<(g-32))))  // sets   bits which are 1 ignores bits which are 0
#define PI_GPIO_CLR(g) (g<32 ? (*(gpio+10)=(1<<g)) : (*(gpio+11)=(1<<(g-32)))) // clears bits which are 1 ignores bits which are 0
#define PI_READ_GPIO(g) (g<32 ? ((*(gpio+13)&(1<<g))?1:0) : ((*(gpio+14)&(1<<(g-32)))?1:0)) // read pin values

#define GPPUD (gpio+37)
#define GPPUDCLKA (gpio+38)		// GPIOs 0-31

struct timespec bitDelay;

#endif
void setup_gpio();

int main(int argc, char *argv[])
{
	uint8_t tblent;				// table entry
	int entry;

	if (argc != 2) {
		fprintf(stderr, "Usage %s <n>, where n is the frequency ID 1-256, see documentation for details\n", argv[0]);
		return(0);
	}

	// get the table entry and do it
	sscanf(argv[1], "%d", &entry);

	if ((entry >= 1) && (entry <= 256)) {
		tblent = (uint8_t)((entry - 1) & 0xff);
		setup_gpio();
		doTableEntry(tblent);
		return(0);
	}

	fprintf(stderr, "Entry must be in the range of 1 to 255\r\n");
}

// Set up a memory regions to access GPIO
void setup_gpio()
#if __PI_ZERO
{
	int  mem_fd;
	void *gpio_map;

	// open /dev/mem 
	if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		printf("can't open /dev/mem \n");
		exit(-1);
	}

	// mmap GPIO 
	gpio_map = mmap(
		NULL,             //Any adddress in our space will do
		BLOCK_SIZE,       //Map length
		PROT_READ | PROT_WRITE,// Enable reading & writting to mapped memory
		MAP_SHARED,       //Shared with other processes
		mem_fd,           //File to map
		GPIO_BASE         //Offset to GPIO peripheral
	);

	close(mem_fd); //No need to keep mem_fd open after mmap

	if (gpio_map == MAP_FAILED) {
		printf("mmap error %d\n", errno);//errno also set!
		exit(-1);
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;

	*GPPUDCLKA = 0;
	*GPPUD = 1;							// 0 off, 1 pull-down, 2 pull-up
	delay();

	*GPPUDCLKA = (1 << PHY_TX_CLK) | (1 << PHY_TX_DATA) | (1 << PHY_TX_ENA) | (1 << PHY_RX_CLK) | (1 << PHY_RX_DATA) | (1 << PHY_RX_ENA) | (1 << PHY_TX_LD) | (1 << PHY_RX_LD);
	delay();
	*GPPUDCLKA = 0;
	delay();

	// set GPIO pins to inputs
	PI_INP_GPIO(PHY_TX_CLK);
	PI_INP_GPIO(PHY_TX_DATA);
	PI_INP_GPIO(PHY_TX_ENA);

	PI_INP_GPIO(PHY_RX_CLK);
	PI_INP_GPIO(PHY_RX_DATA);
	PI_INP_GPIO(PHY_RX_ENA);

#if __TEST_MODE
	for(;;)	{
		PI_GPIO_SET(PHY_TX_ENA);
		delay();
		PI_GPIO_CLR(PHY_TX_ENA);
		delay();
	}
#endif


} // setup_gpio
#endif
#if __PC
// not in a pi
{
	fprintf(stdout, "Setup gpio called\n");
	return;
}
#endif

void setLogGpio(uint8_t logPin, uint8_t value)
#if __PI_ZERO
{
	
	switch (logPin) {

	case LOG_TX_CLK:
		if (value)
			PI_GPIO_SET(PHY_TX_CLK);
		else
			PI_GPIO_CLR((PHY_TX_CLK));
		break;

	case LOG_TX_DATA:
		if (value)
			PI_GPIO_SET(PHY_TX_DATA);
		else
			PI_GPIO_CLR((PHY_TX_DATA));
		break;

	case LOG_TX_LE:
		if (value)
			PI_GPIO_SET(PHY_TX_ENA);
		else
			PI_GPIO_CLR((PHY_TX_ENA));
		break;

	case LOG_RX_CLK:
		if (value)
			PI_GPIO_SET(PHY_RX_CLK);
		else
			PI_GPIO_CLR((PHY_RX_CLK));
		break;

	case LOG_RX_DATA:
		if (value)
			PI_GPIO_SET(PHY_RX_DATA);
		else
			PI_GPIO_CLR((PHY_RX_DATA));
		break;

	case LOG_RX_LE:
		if (value)
			PI_GPIO_SET(PHY_RX_ENA);
		else
			PI_GPIO_CLR((PHY_RX_ENA));
		break;
	}
}

// set a logical pin direction
void setLogGpioDir(uint8_t logPin, uint8_t direction)
{
	uint8_t phypin=0;
	
	switch (logPin) {

	case LOG_TX_CLK:
		phypin = PHY_TX_CLK;
		break;

	case LOG_TX_DATA:
		phypin = PHY_TX_DATA;
		break;

	case LOG_TX_LE:
		phypin = PHY_TX_ENA;
		break;

	case LOG_RX_CLK:
		phypin = PHY_RX_CLK;
		break;

	case LOG_RX_DATA:
		phypin = PHY_RX_DATA;
		break;

	case LOG_RX_LE:
		phypin = PHY_RX_ENA;
		break;
	}
	
	// set the pin direction
	if(direction == LOG_PIN_INPUT)
		PI_INP_GPIO(phypin);
	else
		PI_OUT_GPIO(phypin);
}

#endif
#if __PC
{
	switch (logPin) {

	case LOG_TX_CLK:
#if __VERBOSE
		fprintf(stdout,"TX_CLK %d\n", value);
#endif
		break;

	case LOG_TX_DATA:
#if __VERBOSE
		fprintf(stdout, "TX_DATA %d\n", value);
#else
		fprintf(stdout, "%d", value);
#endif
		break;

	case LOG_TX_LE:
		fprintf(stdout, "\nTX_LE %d\n", value);
		break;

	case LOG_RX_CLK:
#if __VERBOSE
		fprintf(stdout, "RX_CLK %d\n", value);
#endif
		break;

	case LOG_RX_DATA:
#if __VERBOSE
		fprintf(stdout, "RX_DATA %d\n", value);
#else
		fprintf(stdout, "%d", value);
#endif
		break;

	case LOG_RX_LE:
		fprintf(stdout, "\nRX_LE %d\n", value);
		break;
	}
}
#endif

void delay(void)
#if __PI_ZERO
{
	bitDelay.tv_sec = 0;
	bitDelay.tv_nsec = (long)(5000);		// delay in nanoseconds
	nanosleep(&bitDelay, 0);
}

// PI Zero has no LED's
void setLED(uint8_t led, uint8_t value)
{
}

#endif
#if __PC
{
	return;
}

void setLED(uint8_t led, uint8_t value)
{
	switch (led) {

	case TX_LED:
		fprintf(stdout, "Tx LED ");
		break;

	case RX_LED:
		fprintf(stdout, "Rx LED ");
		break;
	}

	switch(value)	{

	case LED_OFF:
		fprintf(stdout, "Off\n");
		break;

	case LED_BLINK:
		fprintf(stdout, "Blink\n");
		break;

	case LED_ON:
		fprintf(stdout, "On\n");
		break;

	}
}
#endif
