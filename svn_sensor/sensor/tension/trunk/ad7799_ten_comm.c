/******************************************************************************
* File Name          : ad7799_ten_comm.c
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for operating AD7799_1 for winch tension
*******************************************************************************/

#include "spi1sad7799_ten.h"
#include "PODpinconfig.h"
#include "ad7799_ten_comm.h"
#include "DTW_counter.h"

#define AD7799OTWEAK	15300		// Tweak the zero offset 

extern uint32_t sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* Pointers point to address upon completion of operation */
extern void 	(*spi1_ten_writedoneptr)(void);	// Address of function to call upon completion of write
extern void 	(*spi1_ten_readdoneptr)(void);	// Address of function to call upon completion of read


uint8_t ad7799_ten_8bit_reg;		// Byte with latest status register from ad7799_1
uint8_t ad7799_ten_8bit_wrt;		// Byte to be written
volatile uint8_t ad7799_ten_comm_busy;	// 0 = not busy with a sequence of steps, not zero = busy
unsigned short ad7799_ten_16bit_wrt;	// 16 bits to be written

/* These 'unions' are used to read bytes into the correct order for mult-byte registers.
The AD7799 sends the data MSB first, so each byte needs to be stored in reverse order */
union SHORTCHAR	ad7799_ten_16bit;
union INTCHAR	ad7799_ten_24bit;

/* Readings and registers for the AD7799. */
struct AD7799REGS ad7799regs[NUMAD7799];
static uint8_t* p8;
static int16_t* p16;
static uint32_t* p32;

/* This holds the last 24 bit register reading, adjusted for bipolar and zero offset, and 
assumes the 24 bit register read was a data and not some other 24 bit register. */
//int32_t	ad7799_ten_last_good_reading;


#define TMAX 64000000*5
/******************************************************************************
 * static int32_t wait_busy(uint32_t ticks);
 * @param	: ticks = number of usec to wait for busy to go to zero
 * @return	: >0 for OK; =<0 timed out
*******************************************************************************/
static int32_t wait_busy(uint32_t ticks)
{
	uint32_t t0 = DTWTIME + ticks * (sysclk_freq/1000000);
	while (  (ad7799_ten_comm_busy != 0) && (((int32_t)DTWTIME - (int32_t)t0) > 0) );
	return ((int32_t)DTWTIME - (int32_t)t0);
}
/******************************************************************************
 * int32_t ad7799_ten_1_initA (uint8_t ucSPS);
 * @brief	: Initialize AD7799_1 registers for strain gauge
 * @param	: ucSPS: Sampling rate code (see ad7799_ten_comm.h file)
 * @return	: 0 = success; -1 = failed (timed out)
*******************************************************************************/
int32_t ad7799_ten_1_initA (uint8_t ucSPS)
{
	/* Select channel with strain gauge, gain 128x, reference voltage detector on */
	//............................... channel....|...gain.....|reference detect.|.bipolar(default)
	ad7799_ten_wr_configuration_reg (AD7799_CH_2 | AD7799_128 |AD7799_REF_DET );
	if (wait_busy(TMAX) <= 0) return -1;

	ad7799_ten_wr_mode_reg(AD7799_ZEROCALIB | (ucSPS & 0x0f) );	// Calibrate chan with inputs internally shorted
	if (wait_busy(TMAX) <= 0) return -1;

	/* Set mode to continuous conversions */
	ad7799_ten_wr_mode_reg(AD7799_IDLE | (ucSPS & 0x0f) );

	return 0;
}

/******************************************************************************
 * void ad7799_ten_wr_comm_rd24b (uint8_t ucX, uint32_t n);
 * @brief	: Write comm register, then read 24b data register (set/exit continuous mode)
 * @param	: uc = Code
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1;
 * @return	: ad7799_ten_comm_busy = 0 when operation completes 
*******************************************************************************/
void ad7799_ten_wr_comm_rd24b1(void);
void ad7799_ten_wr_comm_rd24b2(void);

static uint32_t nsave;

void ad7799_ten_wr_comm_rd24b (uint8_t ucX, uint32_t n)
{
	nsave = n;		// Save for later
	while (ad7799_ten_comm_busy != 0);		// If main did not check we must hang for prev sequence to finish
	ad7799_ten_comm_busy = 1;			// Show interest parties sequence is in progress
	spi1_ten_writedoneptr = &ad7799_ten_wr_comm_rd24b1;// Setup ISR completion address
	ad7799_ten_8bit_wrt = ucX;			// Setup codes to make next operation a read
	spi1_ten_write (&ad7799_ten_8bit_wrt,1);	// Write byte to comm reg
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after write to comm reg is complete */
void ad7799_ten_wr_comm_rd24b1(void)
{
	spi1_ten_readdoneptr = &ad7799_ten_wr_comm_rd24b2;// Next ISR completion address
	spi1_ten_read(&ad7799_ten_24bit.uc[0], 3, 0);	// Start 3 byte read
	return;						// Return to ISR 
}
/* ISR for SPI1 comes here after three bytes were read */
void ad7799_ten_wr_comm_rd24b2(void)
{
	uint8_t ucT = ad7799_ten_24bit.uc[0];		// Swap order of bytes so that 
	ad7799_ten_24bit.uc[0] = ad7799_ten_24bit.uc[2];//  'int32_t' in union is in
	ad7799_ten_24bit.uc[2] = ucT;			//   correct order.
	ad7799regs[nsave].data_reg = ad7799_ten_24bit.n ;// Save in struct
	spi1_ten_writedoneptr = &SPIsd_dummy;		// Set a valid address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;		// Set a valid address to go to when read completes
	ad7799_ten_comm_busy = 0;			// Show interest parties sequence is finished
	return;						// Return to ISR with no operation started
}
/******************************************************************************
 * void ad7799_ten_rd_8bit_reg (uint8_t ucX, uint8_t* p);
 * @brief	: Read an 8 bit register from ad7799
 * @param	: ucX = register code
 * @return	: ad7799_ten_comm_busy = 0 when operation complete 
*******************************************************************************/
void ad7799_ten_rd_8bit_reg1(void);
void ad7799_ten_rd_8bit_reg2(void);


void ad7799_ten_rd_8bit_reg (uint8_t ucX, uint8_t* p)
{
	p8 = p;	// Pointer to AD7799[x] struct.  Save for later
	while (ad7799_ten_comm_busy != 0);			// If main did not check we must hang for prev sequence to finish
	ad7799_ten_comm_busy = 1;				// Show interest parties sequence is in progress
	spi1_ten_writedoneptr = &ad7799_ten_rd_8bit_reg1;	// Setup ISR completion address
	ad7799_ten_8bit_wrt = ((ucX & 0x3f)|0x40);		// Setup codes to make next operation a read
	spi1_ten_write (&ad7799_ten_8bit_wrt,1);		//  Write byte to comm reg
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after byte to comm register has been written */
void ad7799_ten_rd_8bit_reg1(void)
{
	spi1_ten_readdoneptr = &ad7799_ten_rd_8bit_reg2;	// Next ISR completion address
	spi1_ten_read (&ad7799_ten_8bit_reg, 1, 0);		// Read one byte
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after read is complete */
void ad7799_ten_rd_8bit_reg2(void)
{
	spi1_ten_writedoneptr = &SPIsd_dummy;		// Set a dummy address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;		// Set a dummy address to go to when read completes
	*p8 = ad7799_ten_8bit_reg;			// Save reading in struct
	ad7799_ten_comm_busy = 0;			// Show interest parties sequence is finished
	return;	
}
/******************************************************************************
 * void ad7799_ten_rd_status_reg (uint32_t n);
 * @brief	: Read status register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_status_reg (uint32_t n)
{
	ad7799_ten_rd_8bit_reg( (0 << 3), &ad7799regs[n].status_reg );	// Status reg code: 010
	return;
}
/******************************************************************************
 * void ad7799_ten_rd_IO_reg (uint32_t n);
 * @brief	: Read IO register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_IO_reg (uint32_t n)
{
	ad7799_ten_rd_8bit_reg( (5 << 3), &ad7799regs[n].io_reg );	// IO reg code: 101
	return;
}
/******************************************************************************
 * void ad7799_ten_rd_ID_reg (uint32_t n);
 * @brief	: Read ID register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_ID_reg (uint32_t n)
{
	ad7799_ten_rd_8bit_reg( (4 << 3), &ad7799regs[n].id_reg );	// IO reg code: 100
	return;
}
/******************************************************************************
 * void ad7799_ten_rd_configuration_reg (uint32_t n);
 * @brief	: Read 16 bit configuration register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_16bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_configuration_reg (uint32_t n)
{
	ad7799_ten_rd_16bit_reg( (2 << 3), &ad7799regs[n].config_reg );	// Configuration reg code: 010
	return;
}
/******************************************************************************
 * void ad7799_ten_wr_configuration_reg (unsigned short usX);
 * @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit configuration register to ad7799_ten
 * @return	: only hope that it worked.
*******************************************************************************/
void ad7799_ten_wr_configuration_reg (unsigned short usX)
{
	usX &= 0b0011011100110111;	// Some bits *must* be zero for correct operation
	ad7799_ten_16bit.us = usX;		// Save data to be written
	ad7799_ten_wr_16bit_reg( (2 << 3) );	// Configuration reg code: 101
	return;
}
/******************************************************************************
 * void ad7799_ten_rd_mode_reg (uint32_t n);
 * @brief	: Read 16 bit mode register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_16bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_mode_reg (uint32_t n)
{
	ad7799_ten_rd_16bit_reg( (1 << 3), &ad7799regs[n].mode_reg );	// Register selection code: 001
	return;
}
/******************************************************************************
 * void ad7799_ten_rd_offset_reg (uint32_t n);
 * @brief	: Read 24 bit mode register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_24bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_offset_reg (uint32_t n)
{
	ad7799_ten_rd_24bit_reg( (6 << 3), &ad7799regs[n].offset_reg );	// Register selection code: 110
	return;
}
/******************************************************************************
 * void ad7799_ten_wr_offset_reg (int32_t offset);
 * @brief	: Write offset register with some program generated value
 * @param	: offset = value to write into offset register
 * @return	: Operation complete when (ad7799_ten_comm_busy==0) 
*******************************************************************************/
void ad7799_ten_wr_offset_reg (int32_t offset)
{
	ad7799_ten_wr_24bit_reg( (6 << 3), offset);	// Register selection code: 110
	return;
}
/******************************************************************************
 * void ad7799_ten_rd_fullscale_reg (uint32_t n);
 * @brief	: Read 24 bit mode register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_24bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_fullscale_reg (uint32_t n)
{
	ad7799_ten_rd_24bit_reg( (7 << 3), &ad7799regs[n].fullscale_reg );	// Register selection code: 111
	return;
}
/******************************************************************************
 * void ad7799_ten_rd_data_reg (uint32_t n);
 * @brief	: Read 24 bit mode register from ad7799
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @return	: When (ad7799_ten_comm_busy==0) the data is in 'ad7799_ten_24bit' in correct byte order
*******************************************************************************/
void ad7799_ten_rd_data_reg (uint32_t n)
{
	ad7799_ten_rd_24bit_reg( (3 << 3), (uint32_t*)&ad7799regs[n].data_reg );	// Register selection code: 011
	return;
}
/******************************************************************************
 * void ad7799_ten_wr_mode_reg (unsigned short usX);
 * @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit mode register to ad7799
 * @return	: only hope that it worked.
*******************************************************************************/
void ad7799_ten_wr_mode_reg (unsigned short usX)
{
	usX &= 0xf00f;			// Some bits *must* be zero for correct operation
	ad7799_ten_16bit.us = usX;		// Save data to be written
	ad7799_ten_wr_16bit_reg( (1 << 3) );	// Mode reg code: 001
	return;
}
/******************************************************************************
 * void ad7799_ten_set_continous_rd (uint32_t n);
 * @brief	: Writes 0x5c to the comm register to set part in continuous read mode
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 *		: Needs to be followed by a 24b reg read
*******************************************************************************/
void ad7799_ten_set_continous_rd (uint32_t n)
{
	ad7799_ten_wr_comm_rd24b(0x5c,n);		// Set continous read mode
	return;
}
/******************************************************************************
 * void ad7799_ten_exit_continous_rd (uint32_t n);
 * @param	: n = AD7799_1 = 0, AD7799_2 = 1
 * @brief	: Writes 0x58 to the comm register to exit continuous read mode
*******************************************************************************/
void ad7799_ten_exit_continous_rd (uint32_t n)
{
	ad7799_ten_wr_comm_rd24b(0x58,n);		// Exit continous read mode
	return;
}
/******************************************************************************
 * void ad7799_ten_reset (void);
 * @brief	: Send 32 consecutive 1's to reset the digital interface
 * @return	: None now 
*******************************************************************************/
void ad7799_ten_reset1(void);
const uint8_t ad7799_ten_ones[8] = {0xff,0xff,0xff,0xff,0xff};	// 1 bits

void ad7799_ten_reset (void)
{
	while (ad7799_ten_comm_busy != 0);		// If main did not check we must hang for prev sequenc
	ad7799_ten_comm_busy = 1;			// Show interest parties sequence is in progress
	spi1_ten_writedoneptr = &ad7799_ten_reset1;	// Setup ISR completion address
	spi1_ten_write ((uint8_t*)&ad7799_ten_ones[0],5);		// Send 32 all one bits
	return;
}
/* ISR for SPI1 comes here after all bytes have been written */
void ad7799_ten_reset1(void)
{
	spi1_ten_writedoneptr = &SPIsd_dummy;	// Set a valid address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;	// Set a valid address to go to when read completes
	ad7799_ten_comm_busy = 0;		// Show interest parties sequence is finished
	return;					// Return to ISR
}	
/******************************************************************************
 * void ad7799_ten_rd_16bit_reg (uint8_t uc, int16_t* p);
 * @brief	: Read an 16 bit register from ad7799_ten
 * @param	: uc = register code
 * @param	: p = pointer to where the result is to be copied
 * @return	: ad7799_ten_flag is set when sequence completes; data is in 
*******************************************************************************/
void ad7799_ten_rd_16bit_reg1(void);
void ad7799_ten_rd_16bit_reg2(void);
void ad7799_ten_rd_16bit_reg3(void);

void ad7799_ten_rd_16bit_reg (uint8_t uc, int16_t* p)
{
	p16 = p;
	while (ad7799_ten_comm_busy != 0);			// If main did not check we must hang for prev sequenc
	ad7799_ten_comm_busy = 1;				// Show interest parties sequence is in progress
	spi1_ten_writedoneptr = &ad7799_ten_rd_16bit_reg1;	// Setup ISR completion address
	ad7799_ten_8bit_wrt = ((uc & 0x3f)|0x40);		// Setup codes to make next operation a read
	spi1_ten_write (&ad7799_ten_8bit_wrt,1);		// Write byte to comm reg
	return;						// Return to mainline
}
/* ISR for SPI1 comes here after byte to comm register has been written */
void ad7799_ten_rd_16bit_reg1(void)
{
	spi1_ten_readdoneptr = &ad7799_ten_rd_16bit_reg2;// Next ISR completion address
	spi1_ten_read (&ad7799_ten_16bit.uc[0], 2, 0);	// Read high ord byte
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after 2nd byte from specified register has been written */
void ad7799_ten_rd_16bit_reg2(void)
{
	uint8_t ucT = ad7799_ten_16bit.uc[0];
	ad7799_ten_16bit.uc[0] = ad7799_ten_16bit.uc[1];// Swap order of bytes so that 
	ad7799_ten_16bit.uc[1] = ucT;			//     'int16_t' in union is correct.
	spi1_ten_writedoneptr = &SPIsd_dummy;		// Set a valid address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;		// Set a valid address to go to when read completes
	*p16 = ad7799_ten_16bit.us;			// Copy result to destination
	ad7799_ten_comm_busy = 0;			// Show interest parties sequence is finished
	return;						// Return to ISR
}

/******************************************************************************
 * void ad7799_ten_rd_24bit_reg (uint8_t uc, uint32_t* p);
 * @brief	: Read an 24 bit register from ad7799_ten
 * @param	: uc = register selection code
 * @param	: p = pointer to where result is copied
 * @return	: ad7799_ten_comm_busy set to 0 when operation sequence is complete
*******************************************************************************/
void ad7799_ten_rd_24bit_reg1(void);
void ad7799_ten_rd_24bit_reg2(void);
void ad7799_ten_rd_24bit_reg3(void);
void ad7799_ten_rd_24bit_reg4(void);


void ad7799_ten_rd_24bit_reg (uint8_t uc, uint32_t* p)
{
	p32 = p;	// Save where to copy result for later
	while (ad7799_ten_comm_busy != 0);			// If main did not check we must hang for prev sequenc
	ad7799_ten_comm_busy = 1;				// Show interest parties sequence is in progress
	ad7799_ten_24bit.uc[3] = 0;				// This byte is not read into so be sure it is zero
	spi1_ten_writedoneptr = &ad7799_ten_rd_24bit_reg1;	// Setup ISR completion address
	ad7799_ten_8bit_wrt = ((uc & 0x3f)|0x40);		// Setup codes to make next operation a read
	spi1_ten_write (&ad7799_ten_8bit_wrt,1);		// Write byte to comm register
	return;						// Return to mainline
}
/* ISR for SPI1 comes here after byte to comm register has been written */
void ad7799_ten_rd_24bit_reg1(void)
{
	spi1_ten_readdoneptr = &ad7799_ten_rd_24bit_reg2;	// Next ISR completion address
	spi1_ten_read (&ad7799_ten_24bit.uc[0], 3, 0);		// Read high ord byte
	return;						// Return to ISR 
}
/* ISR for SPI1 comes here after 2nd byte from specified register has been written */
void ad7799_ten_rd_24bit_reg2(void)
{
	uint8_t ucT = ad7799_ten_24bit.uc[0];
	ad7799_ten_24bit.uc[0] = ad7799_ten_24bit.uc[2];// Swap order of bytes so that 
	ad7799_ten_24bit.uc[2] = ucT;			//     'int32_t' in union is correct.

	/* Save last good reading, adjusted for biploar and zero offset.  (Assumes 24 bit reading was the data register) */
//	ad7799_ten_last_good_reading = 0x800000 + AD7799OTWEAK- ad7799_ten_24bit.n ;	// Zero = 0x800000 when in bipolar mode;
//	ad7799_ten_last_good_reading = 0x800000 - ad7799_ten_24bit.n ;	// Zero = 0x800000 when in bipolar mode;
//	ad7799_ten_last_good_reading_both[active] = ad7799_ten_last_good_reading;	// Save in currently enabled AD7799
	*p32 = ad7799_ten_24bit.n;
	spi1_ten_writedoneptr = &SPIsd_dummy;		// Set a valid address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;		// Set a valid address to go to when read completes
	ad7799_ten_comm_busy = 0;			// Show interest parties sequence is finished
	return;						// Return to ISR with no operation started
}
/******************************************************************************
 * void ad7799_ten_signextend_24bit_reg (void);
 * @brief	: Convert 3 byte readings to 4 byte signed
*******************************************************************************/
void ad7799_ten_signextend_24bit_reg (void)
{
	ad7799_ten_24bit.s[1] = ad7799_ten_24bit.c[2];		// Sign extend the high order byte
	return;
}

/******************************************************************************
 * void ad7799_ten_wr_16bit_reg (uint8_t uc);
 * @brief	: Write an 16 bit register from ad7799_ten
 * @param	: uc = register code
 * @return	: 'ad7799_ten_comm_busy' flag is set to zero when sequence completes
*******************************************************************************/
void ad7799_ten_wr_16bit_reg1(void);
void ad7799_ten_wr_16bit_reg2(void);
void ad7799_ten_wr_16bit_reg3(void);


void ad7799_ten_wr_16bit_reg (uint8_t uc)
{
	while (ad7799_ten_comm_busy != 0);			// If main did not check we must hang for prev sequenc
	ad7799_ten_comm_busy = 1;				// Show interest parties sequence is in progress
	spi1_ten_writedoneptr = &ad7799_ten_wr_16bit_reg1;	// Setup ISR completion address
	ad7799_ten_8bit_wrt = (uc & 0x3f);			// Setup codes to make next operation a read
	spi1_ten_write (&ad7799_ten_8bit_wrt,1);	// Write byte to comm reg
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after byte to comm register has been written */
void ad7799_ten_wr_16bit_reg1(void)
{
	spi1_ten_writedoneptr = &ad7799_ten_wr_16bit_reg2;	// Next ISR completion address
	spi1_ten_write (&ad7799_ten_16bit.uc[1], 1);		// Write high ord byte
	return;
}
/* ISR for SPI1 comes here after 1st byte from specified register has been written */
void ad7799_ten_wr_16bit_reg2(void)
{
	spi1_ten_writedoneptr = &ad7799_ten_wr_16bit_reg3;	// Next ISR completion address
	spi1_ten_write (&ad7799_ten_16bit.uc[0], 1);		// Write low ord byte
	return;
}
/* ISR for SPI1 comes here after 2nd byte from specified register has been written */
void ad7799_ten_wr_16bit_reg3(void)
{
	spi1_ten_writedoneptr = &SPIsd_dummy;	// Set a valid address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;	// Set a valid address to go to when read completes
	ad7799_ten_comm_busy = 0;		// Show interest parties sequence is finished
	return;					// Return to ISR
}
/******************************************************************************
 * void ad7799_ten_wr_24bit_reg (uint8_t uc, int32_t n);
 * @brief	: Write an 16 bit register from ad7799_ten
 * @param	: uc = register code
 * @param	: n = value to be written
 * @return	: 'ad7799_ten_comm_busy' flag is set to zero when sequence completes
*******************************************************************************/
void ad7799_ten_wr_24bit_reg1(void);
void ad7799_ten_wr_24bit_reg2(void);


void ad7799_ten_wr_24bit_reg (uint8_t uc, int32_t n)
{
	while (ad7799_ten_comm_busy != 0);		// If main did not check we must hang for prev sequenc
	ad7799_ten_comm_busy = 1;			// Show interest parties sequence is in progress
	ad7799_ten_24bit.n = n;				// 24b register value to be written
	spi1_ten_writedoneptr = &ad7799_ten_wr_24bit_reg1;// Setup ISR completion address
	ad7799_ten_8bit_wrt = (uc & 0x3f);		// Setup codes to make next operation a read
	spi1_ten_write (&ad7799_ten_8bit_wrt,1);	// Write byte to comm reg
	return;						// Return to ISR
}
/* ISR for SPI1 comes here after byte to comm register has been written */
void ad7799_ten_wr_24bit_reg1(void)
{
	uint8_t tmp = ad7799_ten_24bit.uc[2];	// Reverse output order of lower 3 bytes
	ad7799_ten_24bit.uc[2] = ad7799_ten_24bit.uc[0];
	ad7799_ten_24bit.uc[0] = tmp;
	
	spi1_ten_writedoneptr = &ad7799_ten_wr_24bit_reg2;	// Next ISR completion address
	spi1_ten_write (&ad7799_ten_24bit.uc[0], 3);		// Write 24b
	return;
}
/* ISR for SPI1 comes here after 2nd byte from specified register has been written */
void ad7799_ten_wr_24bit_reg2(void)
{
	spi1_ten_writedoneptr = &SPIsd_dummy;	// Set a valid address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;	// Set a valid address to go to when read completes
	ad7799_ten_comm_busy = 0;		// Show interest parties sequence is finished
	return;					// Return to ISR
}


