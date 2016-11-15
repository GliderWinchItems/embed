/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : rw_eeprom.c
* Authorship         : deh
* Date First Issued  : 06/03/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : Handles sequence for read/writing eeprogm (using spi1eeprom.c)
*******************************************************************************/
/*
Routines for working with the Microchip 25AA640A eeprom (64Kbit/8Kbyte) on the sensor
board.  See datasheet pdf found in: ../svn_sensor/docs/trunk/datasheets/21830F_eeprom.pdf
*/
#include "spi1eeprom.h"

unsigned int debug1; // Count busy loops for a write

/* Pointer point to address upon completion of operation */
extern void (*spi1_isrdoneptr )(void);	// Address of function to call upon completion of read.

/* These are used during the sequence of spi1 calls */
static volatile int rw_busy_flag = 0;	// Non-zero = read or write sequence is in progress
static char rw_dummy;		// Dummy read-into byte
int eeprom_loopctr;	// Timeout loop counter
static int eeprom_n;		// Number of bytes in eeprom block to write
static char *prw;		// Working pointer into our buffer
static int ctr;			// Current count of bytes to be written
static int peeprom;		// Working address of eeprom

unsigned char eeprom_status_register;	// Status register (last read)

/* eeprom "instructions" (as the spec sheet calls them.  We call them commands...sometimes.) */
static const char wren  = 0x06;	// Set write enable latch: enable
static const char rdsr  = 0x05;	// Read status register
static const char wrdi  = 0x04; // Reset write enable latch: disable
static const char rdcmd = 0x03; // Read
static const char wrcmd = 0x02; // Write
static const char wrsr  = 0x01; // Write status register


/******************************************************************************
 * int rw_eeprom_init(void);
 *  @return	: zero = success; not zero = failed
 *  @brief 	: Initialize SPI1 for eeprom
*******************************************************************************/
int rw_eeprom_init(void)
{
	int err = 0;
	spi1_isrdoneptr = 0;	// jic

	err = spi1eeprom_init();	

	EEPROM_NCS_hi;		// Disable chip select (jic)

	return err;
}
/******************************************************************************
 * int eeprom_busy(void);
 * @brief	: Check if a read or write operation is in progress
 * @return	: zero = not busy. + = busy; -1 = timed out, and abandoned.
******************************************************************************/
int eeprom_busy(void)
{
	return rw_busy_flag;
}
/******************************************************************************
 * unsigned int eeprom_read_status_reg(void);
 * @brief	: Read the status register
 * @return	: high byte = 0 for OK, 0xff bad; low byte = status register
******************************************************************************/
static void eeprom_statusreg1(void);
static void eeprom_statusreg2(void);


unsigned int eeprom_read_status_reg(void)
{
	if (rw_busy_flag != 0) return 0xff00;	// One shouldn't be calling this!
	rw_busy_flag = 1;	// Show that a sequence has started

	/* Send read status register command. */
	spi1_write((char *)&rdsr, 1, &eeprom_statusreg1);	// This sets /CS low

	eeprom_loopctr = 0;	// Timeout counter
	while ((rw_busy_flag != 0) && (eeprom_loopctr++ < 20000));	// Wait until done or timeout
	if (eeprom_loopctr >= 20000) return 0xfe00;	// Return timeout error.
	return (unsigned int)eeprom_status_register;
}
static void eeprom_statusreg1(void)
{ // (UNDER DMA INTERRUPT) Here, the Write Enable has been sent
	/* Send read status register command. */
	spi1_read((char*)&eeprom_status_register, 1, 0x00, &eeprom_statusreg2);
	return;
}
static void eeprom_statusreg2(void)
{ // (UNDER DMA INTERRUPT) Here, the Write Enable has been sent
	EEPROM_NCS_hi;		// Disable chip select
	rw_busy_flag = 0;	// Set not busy, and operation was successful.
	spi1_isrdoneptr = 0; 	//JIC--set DMA interrupt exit address, so that nobody gets called.
	return;
}
/******************************************************************************
 * unsigned int eeprom_write_enable_latch(char onoff);
 * @brief	: Set write-enable-latch on, or off
 * @param	: onoff: 0 = off, not zero = on
 * @return	: 0 = OK; 0xff00 busy; 0xfe00 = timedout
******************************************************************************/
static void eeprom_wren1(void);

unsigned int eeprom_write_enable_latch(char onoff)
{
	if (rw_busy_flag != 0) return 0xff00;	// One shouldn't be calling this!
	rw_busy_flag = 1;	// Show that a sequence has started

	/* Setup command to be used. */
	if (onoff == 0)
		rw_dummy = wrdi;	// Reset write enable latch: disable
	else
		rw_dummy = wren;	// Set write enable latch: enable

	/* Send write status register command. */
	spi1_write(&rw_dummy, 1, &eeprom_wren1);	// This sets /CS low

	eeprom_loopctr = 0;	// Timeout counter
	while ((rw_busy_flag != 0) && (eeprom_loopctr++ < 20000));	// Wait until done or timeout
	if (eeprom_loopctr >= 20000) return 0xfe00;	// Return timeout error.
	return 0;
}
static void eeprom_wren1(void)
{ // (UNDER DMA INTERRUPT) Here, the Write Enable has been sent
	EEPROM_NCS_hi;		// Disable chip select
	rw_busy_flag = 0;	// Set not busy, and operation was successful.
	spi1_isrdoneptr = 0; 	//JIC--set DMA interrupt exit address, so that nobody gets called.
	return;
}
/******************************************************************************
 * unsigned int eeprom_write_status_reg(unsigned char new);
 * @brief	: Write a byte to the status register
 * @return	: 0 = OK; 0xff00 busy; 0xfe00 = timedout
******************************************************************************/
static void eeprom_wrstatusreg1(void);
static void eeprom_wrstatusreg2(void);

unsigned int eeprom_write_status_reg(unsigned char new)
{
	if (rw_busy_flag != 0) return 0xff00;	// One shouldn't be calling this!
	rw_busy_flag = 1;	// Show that a sequence has started

	/* Save byte to be written */
	rw_dummy = new;

	/* Send write status register command. */
	spi1_write((char *)&wrsr, 1, &eeprom_wrstatusreg1);	// This sets /CS low

	eeprom_loopctr = 0;	// Timeout counter
	while ((rw_busy_flag != 0) && (eeprom_loopctr++ < 20000));	// Wait until done or timeout
	if (eeprom_loopctr >= 20000) return 0xfe00;	// Return timeout error.
	return 0;
}
static void eeprom_wrstatusreg1(void)
{ // (UNDER DMA INTERRUPT) Here, the Write Enable has been sent
	/* Send read status register command. */
	spi1_write(&rw_dummy, 1, &eeprom_wrstatusreg2);
	return;
}
static void eeprom_wrstatusreg2(void)
{ // (UNDER DMA INTERRUPT) Here, the Write Enable has been sent
	EEPROM_NCS_hi;		// Disable chip select
	rw_busy_flag = 0;	// Set not busy, and operation was successful.
	spi1_isrdoneptr = 0; 	//JIC--set DMA interrupt exit address, so that nobody gets called.
	return;
}
/******************************************************************************
 * int eeprom_read(u16 address, char *p, int count);
 * @brief	: Read from eeprom
 * @param	: address--eeprom address to start read (0-8191)
 * @param	: p--pointer to buffer 
 * @param	: count--number of bytes to read (1-8192)
 * @return	: zero = OK. not zero = failed initial setup
******************************************************************************/
static void eeprom_read1(void);
static void eeprom_read1_1(void);
static void eeprom_read2(void);
static void eeprom_read3(void);

int eeprom_read(int address, char *p, int count)
{
	volatile int timeout = 0;

	/* Is address valid for eeprom? */
	if (address > 8191)	return 1;

	/* Is count reasonable? */
	if (count > 8192)	return 2;
	if (count < 1)		return 3;

	/* Will read run off the end of the eeprom? */
	if ((address+count-1) > 8192) return 4;

	/* Is eeprom currently busy with an operation? */
	while ((timeout++ < 1000000) && (eeprom_busy() != 0) );
	if (timeout >= 1000000) 	return 5; // Timed out

	/* Save buffer address for next operation in the sequence. */
	prw = p;
	ctr = count;
	peeprom = address;

	/* Setup for read operation */
	EEPROM_NCS_lo;		// Enable chip select

	/* Send command to read. */
	rw_busy_flag = 1;	// Show that a sequence has started
	spi1_write ((char*)&rdcmd, 1, &eeprom_read1); // Send read instruction
	return 0;
}
static void eeprom_read1(void)
{ // (UNDER DMA INTERRUPT) Here, read instruction has been sent.
	/* Send address to start read */
	spi1_write ((char *)(&peeprom)+1, 1, &eeprom_read1_1);
	return;
}
static void eeprom_read1_1(void)
{ // (UNDER DMA INTERRUPT) Here, read instruction has been sent.
	/* Send address to start read */
	spi1_write ((char *)(&peeprom), 1, &eeprom_read2);
	return;
}
static void eeprom_read2(void)
{ // (UNDER DMA INTERRUPT) Here, the address has been sent.
	/* Start the read */
	spi1_read (prw, ctr, 0x00, &eeprom_read3);
	return;
}
static void eeprom_read3(void)
{ // (UNDER DMA INTERRUPT) Here, the bytes of data have been sent.

	/* Disabling chip select terminates the eeprom read execution. */
	EEPROM_NCS_hi;	// Disable chip select

	/* JIC--set DMA interrupt exit address, so that nobody gets called. */
	spi1_isrdoneptr = 0; 
	
	/* Show the mainline that the sequence is complete. */
	rw_busy_flag = 0;	// Show that a sequence has started
		
	return;
}
/******************************************************************************
 * int eeprom_write(u16 address, char *p, int count);
 * @brief	: Write to eeprom
 * @param	: address--eeprom address to start write (0-8191)
 * @param	: p--pointer to buffer 
 * @param	: count--number of bytes to write (1-8192)
 * @return	: zero = OK. not zero = failed initial setup.
******************************************************************************/
static void eeprom_write1(void);
static void eeprom_write2_0(void);
static void eeprom_write2_1(void);
static void eeprom_write2_2(void);
static void eeprom_write2_3(void);
static void eeprom_write3(void);
static void eeprom_write4(void);
static void eeprom_write5(void);
static void eeprom_write6(void);

#define EEPROMTIMEOUT	50000	// Timeout loop counter for waiting for eeprom to complete a write



int eeprom_write(int address, char * p, int count)
{
	volatile int timeout = 0;

	/* Is address valid for eeprom? */
	if (address > 8191)	return 1;

	/* Is count reasonable? */
	if (count > 8192)	return 2;
	if (count < 1)		return 3;

	/* Will read run off the end of the eeprom? */
	if ((address+count-1) > 8192) return 4;

	/* Save a working count of bytes to be written, and pointer to our buffer. */
	ctr = count;	// Number of bytes to write
	prw = p;	// Address of 1st byte in caller's buffer
	peeprom = address;	// Address of 1st byte to write in eeprom

debug1 = 0;

	/* Is eeprom currently busy with an operation? */
	while ((timeout++ < 100000) && (eeprom_busy() != 0) );
	if (timeout >= 100000) return 5; // Timed out

	/* Setup for read operation */
	EEPROM_NCS_lo;		// Enable chip select
	EEPROM_NWP_hi;		// Disable write protect

	/* Enable eeprom write latch.  */
	rw_busy_flag = 1;	// Show that a sequence has started
	spi1_write ((char *)&wren, 1, &eeprom_write1);	// Send command 'wren'
	return 0;
}
static void eeprom_write1(void)
{ // (UNDER DMA INTERRUPT) Here, the Write Enable command has been sent

	/* Raise /CS to "set" the write-enable latch. */
	EEPROM_NCS_hi;	// Disable chip select

	eeprom_loopctr = 0;	// timeout counter initialize

	/* Send read status register command. */
	spi1_write((char *)&rdsr, 1, &eeprom_write2_0);	// This also sets /CS low
	return;
}
static void eeprom_write2_0(void)
{ // (UNDER DMA INTERRUPT) Here, read status register command sent, now read status register.
	/* Read the status register */
	spi1_read((char*)&eeprom_status_register, 1, 0xff, &eeprom_write2_1);
	return;
}
static void eeprom_write2_1(void)
{ // (UNDER DMA INTERRUPT) Here, status register has been read, now send write command
	EEPROM_NCS_hi;			// Disable chip select (which terminates eeprom read)
	/* Check if write enable latch is set */
	if ((eeprom_status_register & 0x02) == 0) // Is Write Enable latch set?
	{ // Here, for some reason the write enable latch bit (WEL) is not set */	
		rw_busy_flag = -2;	// Set not busy, but operation was not successful (code -2)
		spi1_isrdoneptr = 0; 	// JIC--set DMA interrupt exit address, so that nobody gets called.
		return;
	}
	/* Send write command to eeprom. */
	spi1_write((char *)&wrcmd, 1, &eeprom_write2_2);	// This also sets /CS low
	return;
}
static void eeprom_write2_2(void)
{ // (UNDER DMA INTERRUPT) Here, write command has been sent; now send eeprom address, high byte.
	/* Send eeprom address (high byte) to eeprom. */
	spi1_write((char*)(&peeprom)+1, 1, &eeprom_write2_3);
	return;
}
static void eeprom_write2_3(void)
{ // (UNDER DMA INTERRUPT) Here, write command has been sent; now send eeprom address low byte.
	/* Send eeprom address (high byte) to eeprom. */
	spi1_write((char*)(&peeprom), 1, &eeprom_write3);
	return;
}
static void eeprom_write3(void)
{ // (UNDER DMA INTERRUPT) address has been sent; now send the data, if there is more to send.

	/* Writing is within 32 byte blocks in the eeprom, so we need to
           find the number of bytes to write in the current block, etc. */
	int bwb = ((unsigned int)peeprom & 0x001f);	// Byte-within-block address
	eeprom_n = (32 - bwb);		// Max number of bytes left to write in eeprom block

	/* Writes do not cross eeprom block boundaries */
	if (ctr <= eeprom_n)		// Do we have more bytes to write than are left in this eeprom block?
		eeprom_n = ctr; 	// Here, no. We end within the current eeprom block

	/* Write 'eeprom_n' bytes to eeprom out of our buffer ('prw'). */
	spi1_write(prw, eeprom_n, &eeprom_write4);
	return;
}
static void eeprom_write4(void)
{ // (UNDER DMA INTERRUPT) spi write operation complete, now check for eeprom to finish.

	/* Terminate the write, or read status, command */
	EEPROM_NCS_hi;	// Disable chip select (which terminates eeprom write)

	/* Check if eeprom is busy.  Send read status register command. */
	spi1_write ((char *)&rdsr, 1, &eeprom_write5);	// Sets /CS back low.
	return;
}
static void eeprom_write5(void)
{ // (UNDER DMA INTERRUPT) read status register command sent, now read the register.

	/* Read status register */
	spi1_read((char*)&eeprom_status_register, 1, 0xff, &eeprom_write6);
	return;
}
static void eeprom_write6(void)
{ // (UNDER DMA INTERRUPT) read status register complete
	EEPROM_NCS_hi;	// Disable chip select (which terminates eeprom read)

	if ((eeprom_status_register & 0x01) != 0)	// Is a write still in progress?
	{ // Here, yes.  Keep looping through here, until not busy, or timeout counter trips.
		if (eeprom_loopctr++ < EEPROMTIMEOUT)
		{// Send read status register instruction.
			spi1_write ((char *)&rdsr, 1, &eeprom_write5);
			return;
		}
		else
		{ // Here we timed out waiting.
			rw_busy_flag = -1;	// Set not busy, but operation was not successful (code -1)
			spi1_isrdoneptr = 0; 	//JIC--set DMA interrupt exit address, so that nobody gets called.
			return;
		}
	}	
	eeprom_loopctr = 0;	// timeout counter

	/* Here--eeprom has completed the write */

	/* Subtract out the bytes just written from the running count. */
	ctr -= eeprom_n;	// Subtract the number bytes we are writing from the running count.

	/* If we have more bytes to send, then start a new cycle */
	if (ctr > 0)
	{ // Here, more bytes to be written, so start a new write cycle.
debug1 += 1;

		/* Adjust where we are in the sending of the bytes. */
		prw += eeprom_n;	// Advance the pointer in our buffer.
		peeprom += eeprom_n;	// Advance eeprom address

		spi1_write ((char *)&wren, 1, &eeprom_write1); // This also sets /CS low.
		return;
	}
	/* Well done, laddies! */
	EEPROM_NWP_lo;		// Enable write protect
	rw_busy_flag = 0;	// Set not busy, and operation was successful.
	spi1_isrdoneptr = 0; 	//JIC--set DMA interrupt exit address, so that nobody gets called.

	return;
}

