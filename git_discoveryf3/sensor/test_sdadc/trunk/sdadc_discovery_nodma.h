/******************************************************************************
* File Name          : sdadc_discovery_nodma.h
* Date First Issued  : 03/27/2015
* Board              : Discovery F3 w F373 processor
* Description        : SDADC not using DMA 
*******************************************************************************/
/*


*/
#include <stdint.h>

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDADC_DISCOVERY_NODMA
#define __SDADC_DISCOVERY_NODMA

#define NUMBERSDADCS		3	// Max number of SDADCs

#define MAXPORTSPERSDADC	9	// Max nmber of ports per SDADC

#define CIC1DECIMATION		8	// Decimation ratio for first CIC stage
#define CIC2DECIMATION		8	// Decimation ratio for second CIC stage


/* These should match the number of bits on the SDADCx_JCHGR setup,
   which is setup by the SDADCPORTS below. */
#define NUMBERSDADC1		9	// Number of channels: SDADC1
#define NUMBERSDADC2		3	// Number of channels: SDADC2
#define NUMBERSDADC3		9	// Number of channels: SDADC3

/* Bits select SDADC ports for regular sequence */
// Use the table below to select the port bits
//        port number  876543210
#define SDADC1PORTS  0b111111111
#define SDADC2PORTS  0b000000111
#define SDADC3PORTS  0b111111111

/* ** DON'T FORGET TO UNCOMMENT/COMMENT THE PIN SETUP in the .c file */

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

struct SDADCPORT
{
	uint16_t *p;
	uint16_t ct;		// Counter
	uint8_t	idx;		// 0 = 1st half buff ready; 1 = 2nd half ready
	uint8_t	flag;		// 1 = new data ready
};

/*  */
struct SDADCPORTPARM
{
	struct CICLN8M3 *p;		// Working pointer to cic buffer: current
	struct CICLN8M3 *p_next;	// Working pointer to cic buffer: next
	struct CICLN8M3 *p_end;		// End of buffers
	struct CICLN8M3 *p_begin;	// Beginning of buffers
};

/******************************************************************************/
int sdadc_discovery_nodma_init(void);
/* @brief 	: Initialize SDADC, DMA, calibration, etc., then start it running
 * @return	: 0 = OK; negative = failed
*******************************************************************************/
void sdadc_discovery_nodma_request_calib(void);
/* @brief 	: Calibration with be inserted after the current conversion completes
*******************************************************************************/


#endif 
