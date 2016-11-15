/******************************************************************************
* File Name          : ad7799_vcal_comm.c
* Date First Issued  : 10/04/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for operating AD7799_1
*******************************************************************************/
/*
10/04/2015 Hack of svn_pod.../devices/ad7799_vcal_comm.[ch]
*/
#include "spi1ad7799_vcal.h"
#include "PODpinconfig.h"
#include "ad7799_vcal_comm.h"
#include "clockspecifysetup.h"

#define AD7799OTWEAK	15300		// Tweak the zero offset 

/* APB2 bus frequency is used for setting up the divider for SPI1 */
extern unsigned int	pclk2_freq;	/*	SYSCLKX/APB2 divider	E.g. 36000000 	*/


/* Pointers point to address upon completion of operation in 'spi1ad7799_vcal' */
extern void (*spi1_vcal_doneptr)(void);	// Address of function to call upon completion of write
extern void (*spi1_vcal_exti2ptr)(void);	// Low Level interrupt trigger function callback

unsigned char ad7799_vcal_8bit_reg;	// Byte with latest status register from ad7799_vcal_1
unsigned char ad7799_vcal_8bit_wrt;	// Byte to be written
static unsigned char ad7799_vcal_8bit_wrt2; // Byte to write (not above)
//volatile unsigned char ad7799_vcal_comm_busy;// 0 = not busy with a sequence of steps, not zero = busy
unsigned short ad7799_vcal_16bit_wrt;	// 16 bits to be written

/* These 'unions' are used to read bytes into the correct order for mult-byte registers.
The AD7799 sends the data MSB first, so each byte needs to be stored in reverse order */
union SHORTCHAR	ad7799_vcal_16bit;
union INTCHAR	ad7799_vcal_24bit;

/* This holds the last 24 bit register reading, adjusted for bipolar and zero offset, and 
assumes the 24 bit register read was a data and not some other 24 bit register. */
int		ad7799_vcal_last_good_reading;

/******************************************************************************
 * static void ad7799_vcal_wr_comm_reg(unsigned char ucX, void* pnext);
 * @brief	: write to the comm register
 * @param	: ucX = code for comm register
 * @param	: pnext = pointer to next operation upon completion of spi isr 
*******************************************************************************/
static void ad7799_vcal_wr_comm_reg(unsigned char ucX, void* pnext)
{
	spi1ad7799_vcal_push(pnext);			// Setup ISR completion address
	ad7799_vcal_8bit_wrt = (ucX & 0x7c);		// Setup codes for next operation
	spi1_vcal_write (&ad7799_vcal_8bit_wrt,1);	// Write byte to comm reg
	return;						// Return to ISR
}
/******************************************************************************
 * char ad7799_vcal_1_RDY(void);
 * @brief	: Initialize AD7799_1 registers for strain gauge; /CS left in low state
 * @return	: Not zero = AD7799_1 is not ready; zero = data ready
*******************************************************************************/
char ad7799_vcal_1_RDY(void)
{
	/* Select AD7799_1 */
	AD7799_1_CS_low	// Macro is in PODpinconfig.h

	/* PA6 reads the MISO pin level */
	return ( GPIO_IDR(GPIOA) & (1<<6) );
}
/******************************************************************************
 * void ad7799_vcal_rd_8bit_reg (unsigned char);
 * @brief	: Read an 8 bit register from ad7799
 * @param	: register code
 * @return	: ad7799_vcal_flag is set when sequence completes; data is in 
*******************************************************************************/
static void ad7799_vcal_rd_8bit_reg1(void);

void ad7799_vcal_rd_8bit_reg (unsigned char ucX)
{
	ad7799_vcal_wr_comm_reg (((ucX & 0x3f)|0x40),&ad7799_vcal_rd_8bit_reg1);
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after byte to comm register has been written */
static void ad7799_vcal_rd_8bit_reg1(void)
{
	spi1_vcal_read (&ad7799_vcal_8bit_reg, 1, 0x00);	// Read one byte
	return;						// Return to ISR
}
/******************************************************************************
 * void ad7799_vcal_wr_8bit_reg (unsigned char ucX, unsigned char ucW);
 * @brief	: Read an 8 bit register from ad7799
 * @param	: ucX = register code
 * @param	: ucW = byte to write
*******************************************************************************/
static void ad7799_vcal_wr_8bit_reg1(void);

void ad7799_vcal_wr_8bit_reg (unsigned char ucX, unsigned char ucW)
{
	ad7799_vcal_8bit_wrt2 = ucW;		// Save byte to write
	ad7799_vcal_wr_comm_reg( (ucX & ~0x40), &ad7799_vcal_wr_8bit_reg1);
	return;					// Return to ISR
}
/* ISR for SPI1 comes here after byte to comm register has been written */
static void ad7799_vcal_wr_8bit_reg1(void)
{
	spi1_vcal_write (&ad7799_vcal_8bit_wrt2,1);	// Write byte
	return;					// Return to ISR
}
/******************************************************************************
 * void ad7799_vcal_rd_status_reg (void);
 * @brief	: Read status register from ad7799
 * @return	: data is in ad7799_vcal_8bit_reg 
*******************************************************************************/
void ad7799_vcal_rd_status_reg (void)
{
	ad7799_vcal_rd_8bit_reg(AD7799_RD_STATUS_REG);	// Status reg code: 000
	return;
}
/******************************************************************************
 * void ad7799_vcal_rd_IO_rd_reg (void);
 * @brief	: Read IO register from ad7799
 * @return	: data is in ad7799_vcal_8bit_reg 
*******************************************************************************/
void ad7799_vcal_IO_rd_reg (void)
{ // Write comm register, then read one byte IO reg code: 101
	ad7799_vcal_rd_8bit_reg(AD7799_IO_REG); // IO reg code: 101
	return;
}
/******************************************************************************
 * void ad7799_vcal_wr_IO_reg (unsigned char ucW);
 * @brief	: Write byte to IO register
*******************************************************************************/
void ad7799_vcal_wr_IO_reg (unsigned char ucW)
{ // Write comm register, then read one byte IO reg code: 101
	ad7799_vcal_wr_8bit_reg(AD7799_IO_REG, ucW); 	// IO reg code: 101
	return;
}
/******************************************************************************
 * void ad7799_vcal_rd_ID_reg (void);
 * @brief	: Read ID register from ad7799
 * @return	: data is in ad7799_vcal_8bit_reg
*******************************************************************************/
void ad7799_vcal_rd_ID_reg (void)
{
	ad7799_vcal_rd_8bit_reg(AD7799_ID_REG); // IO reg code: 100
	return;
}
/******************************************************************************
 * void ad7799_vcal_rd_configuration_reg (void);
 * @brief	: Read 16 bit configuration register from ad7799
*******************************************************************************/
void ad7799_vcal_rd_configuration_reg (void)
{
	ad7799_vcal_rd_16bit_reg( (2 << 3) );	// Configuration reg code: 010
	return;
}
/******************************************************************************
 * void ad7799_vcal_wr_configuration_reg (uint16_t usX);
 * @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit configuration register to ad7799
 * @return	: only hope that it worked.
*******************************************************************************/
void ad7799_vcal_wr_configuration_reg (uint16_t usX)
{
	usX &= 0b0011011100110111;		// Some bits *must* be zero for correct operation
	ad7799_vcal_16bit.us = usX;		// Save data to be written
	ad7799_vcal_wr_16bit_reg( (2 << 3) );	// Configuration reg code: 101
	return;
}
/******************************************************************************
 * void ad7799_vcal_rd_mode_reg (void);
 * @brief	: Read 16 bit mode register from ad7799
*******************************************************************************/
void ad7799_vcal_rd_mode_reg (void)
{
	ad7799_vcal_rd_16bit_reg( (1 << 3) );	// Register selection code: 001
	return;
}
/******************************************************************************
 * void ad7799_vcal_rd_offset_reg (void);
 * @brief	: Read 24 bit mode register from ad7799
*******************************************************************************/
void ad7799_vcal_rd_offset_reg (void)
{
	ad7799_vcal_rd_24bit_reg( (6 << 3) );	// Register selection code: 110
	return;
}
/******************************************************************************
 * void ad7799_vcal_rd_fullscale_reg (void);
 * @brief	: Read 24 bit mode register from ad7799
*******************************************************************************/
void ad7799_vcal_rd_fullscale_reg (void)
{
	ad7799_vcal_rd_24bit_reg( (7 << 3) );	// Register selection code: 111
	return;
}
/******************************************************************************
 * void ad7799_vcal_rd_data_reg (void);
 * @brief	: Read 24 bit mode register from ad7799
 * @return	: last_good_reading--'ad7799_vcal_24bit' in correct byte order
*******************************************************************************/
void ad7799_vcal_rd_data_reg (void)
{
	ad7799_vcal_rd_24bit_reg( (3 << 3) );	// Register selection code: 011
	return;
}

/******************************************************************************
 * void ad7799_vcal_wr_mode_reg (unsigned short usX);
 * @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit mode register to ad7799
 * @return	: only hope that it worked.
*******************************************************************************/
void ad7799_vcal_wr_mode_reg (unsigned short usX)
{
	usX &= 0xf00f;				// Some bits *must* be zero for correct operation
	ad7799_vcal_16bit.us = usX;		// Save data to be written
	ad7799_vcal_wr_16bit_reg( (1 << 3) );	// Mode reg code: 001
	return;
}
/******************************************************************************
 * void ad7799_vcal_set_continous_rd (void);
 * @brief	: Writes 0x5c to the comm register to set part in continuous read mode
*******************************************************************************/
void ad7799_vcal_set_continous_rd (void)
{
	spi1ad7799_vcal_push(&ad7799_vcal_rd_data_reg);
	ad7799_vcal_8bit_wrt = (0x5c);		// 
	spi1_vcal_write (&ad7799_vcal_8bit_wrt,1);	// Write byte to comm reg
	return;
}
/******************************************************************************
 * void ad7799_vcal_exit_continous_rd (void);
 * @brief	: Writes 0x58 to the comm register to exit continuous read mode
*******************************************************************************/
void ad7799_vcal_exit_continous_rd (void)
{
	spi1ad7799_vcal_push(&ad7799_vcal_rd_data_reg);
	ad7799_vcal_8bit_wrt = (0x58);		// 
	spi1_vcal_write (&ad7799_vcal_8bit_wrt,1);	// Write byte to comm reg
	return;
}
/******************************************************************************
 * void ad7799_vcal_vcal_reset(void);
 * @brief	: Send 32 consecutive 1's to reset the digital interface
 * @return	: None now 
*******************************************************************************/
const unsigned char ad7799_vcal_ones[8] = {0xff,0xff,0xff,0xff};	// 1 bits
static void ad7799_vcal_reset1(void);
static volatile uint16_t sw_loop = 0;	// Wait for completion switch

void ad7799_vcal_reset(void)
{
	spi1ad7799_vcal_push(&ad7799_vcal_reset1);	// Setup ISR completion address
	spi1_vcal_write ((unsigned char*)&ad7799_vcal_ones[0],4);// Send 32 all one bits
	sw_loop = 1;		// Show write is busy
	while (sw_loop != 0);	// Wait for spi interrupt (below) to clear

	/* Upon completion of the reset wait for more than 500 us, e.g. 1 ms! */
	volatile uint32_t tick = *(volatile unsigned int *)0xE0001004 + (sysclk_freq/1000);
	while (( (int)tick ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 );

	return;
}
static void ad7799_vcal_reset1(void)
{
	sw_loop = 0;	// Reset busy switch
	return;		// Return with no new function set up.
}
/******************************************************************************
 * void ad7799_vcal_rd_16bit_reg (uint8_t uc);
 * @brief	: Read an 16 bit register from ad7799
 * @param	: uc = register code (RS2.RS1,RS0)
 * @return	: ad7799_vcal_flag is set when sequence completes; data is in 
*******************************************************************************/
static void ad7799_vcal_rd_16bit_reg1(void);

void ad7799_vcal_rd_16bit_reg (uint8_t uc)
{
			// If main did not check we must hang for prev sequenc
	spi1ad7799_vcal_push(&ad7799_vcal_rd_16bit_reg1);// Setup ISR completion address
	ad7799_vcal_8bit_wrt = ((uc & 0x3f)|0x40);	// Setup codes to make next operation a read
	spi1_vcal_write (&ad7799_vcal_8bit_wrt,1);	// Write byte to comm reg
	return;						// Return to mainline
}
/* ISR for SPI1 comes here after byte to comm register has been written */
static void ad7799_vcal_rd_16bit_reg1(void)
{
//	spi1ad7799_vcal_push(&ad7799_vcal_rd_16bit_reg2);// Next ISR completion address
	spi1_vcal_read (&ad7799_vcal_16bit.uc[0], 2, 0x00);// Read two bytes
	return;						// Return to ISR
}
/******************************************************************************
 * uint16_t ad7799_vcal_rd_16bit_swab(void);
 * @brief	: Put received bytes in little endian order
 * @return	: 16b register in usable byte order
*******************************************************************************/
uint16_t ad7799_vcal_rd_16bit_swab(void)
{
	unsigned char ucT = ad7799_vcal_16bit.uc[0];
	ad7799_vcal_16bit.uc[0] = ad7799_vcal_16bit.uc[1];// Swap order of bytes so that 
	ad7799_vcal_16bit.uc[1] = ucT;			// 'int' in union is correct.
	return ad7799_vcal_16bit.us;			// Return to ISR
}
/******************************************************************************
 * void ad7799_vcal_rd_24bit_reg (unsigned char ucX);
 * @brief	: Read an 24 bit register from ad7799
 * @param	: ucX = register selection code
*******************************************************************************/
static void ad7799_vcal_rd_24bit_reg1(void);
//static void ad7799_vcal_rd_24bit_reg2(void);

void ad7799_vcal_rd_24bit_reg (unsigned char ucX)
{
	ad7799_vcal_24bit.uc[3] = 0;			// This byte is not read into so be sure it is zero
	spi1ad7799_vcal_push(&ad7799_vcal_rd_24bit_reg1);// Next ISR completion address
	ad7799_vcal_8bit_wrt = ((ucX & 0x7c)|0x40);	// Setup codes for next operation
	spi1_vcal_write (&ad7799_vcal_8bit_wrt,1);	// Write byte to comm reg
	return;						// Return to mainline
}
/* ISR for SPI1 comes here after byte to comm register has been written */
static void ad7799_vcal_rd_24bit_reg1(void)
{
//	spi1ad7799_vcal_push(&ad7799_vcal_rd_24bit_reg2);// Next ISR completion address
	spi1_vcal_read(&ad7799_vcal_24bit.uc[0], 3, 0x00);	// Read high ord byte
	return;	
}
/******************************************************************************
 * int32_t ad7799_vcal_rd_24bit_polar(uint16_t us);
 * @brief	: Read an 24 bit register from ad7799
 * @param	: us = unipolar/bipolar: 0 = unipolar, not zero = bipolar
 * @return	: int32_t with fixed up reading
*******************************************************************************/
/* ISR for SPI1 comes here after 2nd byte from specified register has been written */
int32_t ad7799_vcal_rd_24bit_polar(uint16_t us)
{
	unsigned char ucT = ad7799_vcal_24bit.uc[0];
	ad7799_vcal_24bit.uc[0] = ad7799_vcal_24bit.uc[2];// Swap order of bytes so that 
	ad7799_vcal_24bit.uc[2] = ucT;			// 'int' in union is correct.

	if (us == 0)
	{ // Here, bit clear = bipolar; Zero = 0x800000 when in bipolar mode
		ad7799_vcal_last_good_reading = (0x800000 + AD7799OTWEAK- ad7799_vcal_24bit.n);	
		return ad7799_vcal_last_good_reading;
	}
	ad7799_vcal_last_good_reading = ad7799_vcal_24bit.n; // JIC legacy code uses
	return ad7799_vcal_last_good_reading; 
}
/******************************************************************************
 * void ad7799_vcal_signextend_24bit_reg (void);
 * @brief	: Convert 3 byte readings to 4 byte signed
*******************************************************************************/
void ad7799_vcal_signextend_24bit_reg (void)
{
	ad7799_vcal_24bit.s[1] = ad7799_vcal_24bit.c[2];		// Sign extend the high order byte
	return;
}
/******************************************************************************
 * void ad7799_vcal_wr_16bit_reg(unsigned char);
 * @brief	: Write an 16 bit register from ad7799
 * @param	: register code
*******************************************************************************/
static void ad7799_vcal_wr_16bit_reg1(void);
static void ad7799_vcal_wr_16bit_reg2(void);

void ad7799_vcal_wr_16bit_reg(unsigned char ucX)
{
	ad7799_vcal_wr_comm_reg((ucX & ~0x40), &ad7799_vcal_wr_16bit_reg1);
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after byte to comm register has been written */
static void ad7799_vcal_wr_16bit_reg1(void)
{
	spi1ad7799_vcal_push(&ad7799_vcal_wr_16bit_reg2);// Next ISR completion address
	spi1_vcal_write (&ad7799_vcal_16bit.uc[1], 1);	// Write high ord byte
	return;
}
/* ISR for SPI1 comes here after 1st byte from specified register has been written */
static void ad7799_vcal_wr_16bit_reg2(void)
{	// Here, don't push a new address.  Let the SPI completion take the next higher address on the stack. */
	spi1_vcal_write (&ad7799_vcal_16bit.uc[0], 1);	// Write low ord byte
	return;
}

