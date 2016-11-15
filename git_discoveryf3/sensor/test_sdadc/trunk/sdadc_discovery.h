/******************************************************************************
* File Name          : sdadc_discovery.h
* Date First Issued  : 01/20/2015
* Board              : Discovery F3 w F373 processor
* Description        : Initial shakeout of SDADC usage
*******************************************************************************/
/*
Example of timing/rates--

At sysclk = 64MHz, divided by 12 to get SDADC clock = 5 1/3 MHz--
one SDADC clock                     = 0.1875 us
360 SDADC clocks for one conversion = 67.5 us
nine conversions for one sequence   = 607.5 us
eight, nine conversion, sequences   = 4.860 ms per DMA interrupt
filter decimation of 64 * 607.5     = 33.880 ms per filtered output

*/
#include <stdint.h>

/* Define to prevent recursive inclusion */
#ifndef __SDADC_DISCOVERY
#define __SDADC_DISCOVERY

#define NUMBERSDADCS		3	// Number of SDADCs

#define MAXPORTSPERSDADC	9	// Max number of ports SDADC supports

// The DMA stores each channel conversion in a scan sequence, and the scan repeats.
// When the 1/2 way point is stored the DMA interrupts.  When the end is stored the
// DMA interrupts and starts at the beginning of the buffer.
#define NUMBERSEQ1		8	// Number of sequences in 1/2 of the buffer: SDADC1
#define NUMBERSEQ2		24	// Number of sequences in 1/2 of the buffer: SDADC2
#define NUMBERSEQ3		8	// Number of sequences in 1/2 of the buffer: SDADC3

/* Number of ports scanned for each SDADC.  These should match the number
   of bits on the SDADCx_JCHGR setup, which is setup by the SDADCPORTS below. */
#define NUMBERSDADC1		9	// Number of channels: SDADC1
#define NUMBERSDADC2		3	// Number of channels: SDADC2
#define NUMBERSDADC3		9	// Number of channels: SDADC3

/* Bits select SDADC ports for injected sequence */
// Use the table below to select the port bits
// If all bits are zero the SDADC is setup, but not started.
//        port number  876543210
#define SDADC1PORTS  0b111111111
#define SDADC2PORTS  0b000000111
#define SDADC3PORTS  0b111111111

/* ** DON'T FORGET TO UNCOMMENT/COMMENT THE PIN SETUP in the .c file ** */

// '*' = pin can be used either with SDADC1 or SDADC2
// Column 'A' each SDADC can have 9 single-ended inputs, but a few are shared (see *)
// Column 'B' 8P8C connector pin number (connector 1 & 3) 6P6C (connector 2)
// Column 'chan' is SDADC channel/port number
	// SDADC1 
//                       A  B port
	// GPIOB, 0,  // 1  8  6
	// GPIOB, 1,  // 2  7  5
	// GPIOB, 2,  //*3  6  4
	// GPIOE, 7,  //*4  5  3
	// GPIOE, 8,  // 5  4  8
	// GPIOE, 9,  //*6  3  7
	// GPIOE, 10, // 7  2  2
	// GPIOE, 11, //*8  1  1
	// GPIOE, 12, //*9  -  0

	// SDADC2
	// GPIOB, 2,  //*3  -  6
	// GPIOE, 7,  //*4  -  5
	// GPIOE, 8,  // 5  -  8
	// GPIOE, 9,  //*6  -  7
	// GPIOE, 11, //*8  -  4
	// GPIOE, 12, //*9  2  3
	// GPIOE, 13, // 1  3  2
	// GPIOE, 14, // 2  5  1
	// GPIOE, 15, // 3  4  0

	// SDADC3
	// GPIOB, 14, // 1  8  8
	// GPIOB, 15, // 2  7  7
	// GPIOD, 8,  // 3  6  6
	// GPIOD, 9,  // 4  5  5
	// GPIOD, 10, // 5  4  4
	// GPIOD, 11, // 6  3  3
	// GPIOD, 12, // 7  2  2
	// GPIOD, 13, // 8  1  1
	// GPIOD, 14, // 9  -  0

/* Variables associated with SDADC. */
struct SDADCVAR
{
	uint32_t recalib_ld;	// Re-calibration re-load count
	uint32_t recalib_ctr;	// Re-calibration count-down counter
	uint16_t recalib_n;	// Re-calibration count of recalibrations
	uint8_t	idx;		// 0 = 1st half buff ready; 1 = 2nd half ready
	uint8_t	flag;		// 1 = new data ready
};

/* The following is fixed data for each SDADC w DMA configuration */
struct SDADCFIX
{
	int16_t  *base;		// Pointer to double buffer base
	uint32_t sdbase;	// SDADC base address
	struct SDADCVAR *var;	// Pointer to struct with variables
	uint8_t	llvl_irq;	// Low Level IRQ number
	uint8_t dmachan;	// DMA channel number (3-5)
	uint32_t dmamskh;	// DMA ISR mask: Half buffer flag
	uint32_t dmamska;	// DMA ISR mask: All flags
	uint8_t	sdnum;		// SDADC number - 1 (0-2)
	uint16_t size;		// Number of dma items
	uint8_t llvl_priority;	// Low level interrupt priority
	uint8_t dma_priority;	// DMA channel priority
	uint8_t dma_irq_num;	// DMA nvic number
	uint16_t nseq;		// Number of sequences in 1/2 buffer
	uint16_t nadc;		// Number of ADCs in a sequence
};

/******************************************************************************/
int sdadc_discovery_init(void);
/* @brief 	: Initialize SDADC, DMA, calibration, etc., then start it running
 * @return	: 0 = OK; negative = failed
*******************************************************************************/
void sdadc_request_calib(void);
/* @brief 	: Calibration with be inserted after the current conversion completes
*******************************************************************************/
 void sdadc_set_recalib_ct(uint8_t sdnum, uint32_t count);
/* @brief 	: Loads a new count into the re-calibration counter
 * @param	: sdnum = use SDADC number (1,2,or 3) to count sequences
 * @param	: count = number of *dma interrupts* of ' SDADC sdnum' between re-calibrations 
 * @param	:         0 = no recalibrations
*******************************************************************************/

/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
extern void 	(*sdadc1_ll_ptr)(const struct SDADCFIX *p);	// Point processing routine
extern void 	(*sdadc2_ll_ptr)(const struct SDADCFIX *p);	// Point processing routine
extern void 	(*sdadc3_ll_ptr)(const struct SDADCFIX *p);	// Point processing routine

extern struct SDADCVAR sdadcvar[NUMBERSDADCS];
extern const struct SDADCFIX sdadcfix[NUMBERSDADCS];

#endif 
