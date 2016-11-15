/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_comm.c
* Hackerees          : deh
* Date First Issued  : 07/12/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for communicating with the SD card (using spi2card)
*******************************************************************************/

#include "spi2card.h"
#include "PODpinconfig.h"

/* Debuggers */
unsigned int uiDebug10;	



unsigned char sdcard_comm_busy;		// sdcard comm sequence in progress: 0 = not busy, 1 = busy

#define SDTOKEN	0xFE	// Signifies end of data to SD card

/* Power on reset is at least 74 consecutive "one's" ("empty" bytes in sd card parlance) sent to the SD card */
#define SDRESETSIZE	10
const char sdcardreset[SDRESETSIZE] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

struct SDCARDCMD sdcardcmd;	// Command buffer

/******************************************************************************
 * void sdcard_init(unsigned char *pBlkZero, unsigned int uiBlkZeroSize);
 * @param	: Pointer to buffer to hold block zero
 * @brief	: Do the initial setup for the SD Card. Get block zero data
*******************************************************************************/
void sdcard_init(unsigned char *pBlkZero, unsigned int uiBlkZeroSize)
{
	while (spi2_busy_comm_busy != 0);	// JIC!
	sdcard_comm_busy = 1;			// Show interest parties sequence is in progress
	/* Reset is done by holding /CS high and sending at least 74 consecutive 1's */
	SDCARD_CS_high				// Macro is in ../devices/PODpinconfig.h
	spi2_write (&sdcardreset, SDRESETSIZE, &sdcard_init1);	// Start write of reset
	return;					// Return to caller
}
/* ISR for dma1ch5 (spi2 write) comes here after 'sdcardreset' has been sent */
void sdcard_init1(void)
{
	while (spi2_busy_comm_busy != 0) 	// We should not have to loop
uiDebug10 += 1;	// Just to check that the spi wasn't busy when the dma isr completed

	/* From now on the /CS should be held low */		
	SDCARD_CS_lo				// Macro is in ../devices/PODpinconfig.h

	/* Send the initial command */
	/* (I got this from the HC12 .s file stuff) */
	sdcardcmd.add = 0;		// Block 0 has the info about the card
	sdcardcmd.crc = 0x95;		// Last bit of CRC7 byte is always one(?)
	spi2_write (&sdcardreset, SDRESETSIZE, &sdcard_init2);	// Start write of reset

	return;					// Return to ISR
}
/* ISR for dma1ch5 (spi2 write) comes here after 'sdcardcmd' has been sent */
void sdcard_init2(void)
{
	spi2_writedoneptr = 0;			// To be safe ISR write completion address to null
	spi2_read (pBlkZero, uiBlkZeroSize, &sdcard_init3);	// Start write of reset
	return;
}
/* ISR for dma1ch5 (spi2 write) comes here after 'sdcardcmd' has been sent */
void sdcard_init3(void)
{
	spi2_readdoneptr = 0;			// Set ISR read complete call address back to null
	sdcard_comm_busy = 0;			// Show interest parties sequence is finished
	return;
}
/******************************************************************************
 * void sdcard_init(unsigned char cmd);
 * @param	: Byte with command code
 * @brief	: Do the initial setup for the SD Card
*******************************************************************************/
void sdcard_sendcmd(unsigned char cmd)
{



	return;
}

