/*****************************************************************************
* File Name          : lis302.c
* Date First Issued  : 01/21/2015
* Board              : Discovery F4
* Description        : ST LIS302 accelerometer operation
*******************************************************************************/
/*

*/

#include "common.h"
#include "lis302.h"
#include "countdowntimer.h"
#include "spi1rwB.h"

#define ZYXDA	0x08	// x,y,z data is ready.

/* Subroutine prototypes. */
static void lis302_spi(void);
static void lis302_timer(void);

// This holds the address of the function that will be called
void 	(*timer_sw_ptr2)(void) = 0;	// Function pointer for chaining

/* Write ctl reg1: 400 Hz, Power ON, XYZ enabled */
static const char wrt_reg1[2] = {0x20, 0xC7};

/* Read, increment address:  Status reg and x, y, z */
#define SPI1READSIZE	7
static const char read_xyz[SPI1READSIZE] = {0xE7};

static int8_t spi_in[8]; 	// spi incoming bytes

#define LISBUFSIZE	16	
static struct LIS302AVE lisbuf[LISBUFSIZE];
static uint16_t Idxin = 0;
static uint16_t Idxout = 0;

/******************************************************************************
 * static uint16_t adv_idx (uint16_t idx);
 *  @brief 
*******************************************************************************/ 
static uint16_t adv_idx (uint16_t idx)
{
	idx += 1; 
	if (idx >= LISBUFSIZE) idx = 0;
	return idx;
}

/******************************************************************************
 * void lis302_init(void);
 *  @brief	: Initialize the mess
*******************************************************************************/ 
void lis302_init(void)
{
	/* Initialize SPI1 for accelerometer */
	spi1rw_initB();	// 

	/* Setup control register */
	spi1_rwB((char*)wrt_reg1,(char*)spi_in, 2);

	/* SPI complete will go to the following address. */
	spi1_readdoneptrB = &lis302_spi; // SPI callback address

	/* Timer will go to the following address. And start reading. */
	timer_sw_ptr = &lis302_timer;

	/* Get time started. */
	timer_debounce_init();

	return;
}
/******************************************************************************
 * struct LIS302AVE *lis302_get(void);
 * @brief	: Get xyz data if ready.
 * @return	: NULL = no data, or ptr to data
*******************************************************************************/ 
struct LIS302AVE *lis302_get(void)
{
	struct LIS302AVE *p;
	if (Idxin == Idxout) return 0;
	p = &lisbuf[Idxout];
	Idxout = adv_idx(Idxout);
	return p;
}

/* ####################################################################
   Call back from SPI routine when operation is complete
   ###################################################################$ */
static void lis302_spi(void)
{ // Here, SPI has completed the read sequence.
	if ( (spi_in[0] & ZYXDA) != 0) // New LIS302 data ready?
	{ // Here, new data are ready
		/* Sum x, y, z readings and count. */
		lisbuf[Idxin].x += spi_in[2]; // X
		lisbuf[Idxin].y += spi_in[4]; // Y
		lisbuf[Idxin].z += spi_in[6]; // Z
		lisbuf[Idxin].ct += 1;	// Count for averaging
		if (lisbuf[Idxin].ct >= LISAVECT)
		{ // Here, end of averaging.
			Idxin = adv_idx(Idxin); // Advance buff index
			lisbuf[Idxin].ct = 0;	// Reset for next round
			lisbuf[Idxin].x  = 0;
			lisbuf[Idxin].y  = 0;
			lisbuf[Idxin].z  = 0;
		}
	}
	return;
}
/* ####################################################################
   Call back from timer routine
   ###################################################################$ */
static void lis302_timer(void)
{
	/* Start a new SPI read */
	if (spi1_busyB() != 0) // JIC (it should always not be busy)
	{ // Here, SPI is not busy.
		/* Start another read operation. */
		spi1_rwB((char*)read_xyz, (char*)spi_in, SPI1READSIZE);
	}

	/* Call other routines if an address is set up */
	if (timer_sw_ptr2 != 0)	
		(*timer_sw_ptr2)();	// Call a function.

	return;
}

