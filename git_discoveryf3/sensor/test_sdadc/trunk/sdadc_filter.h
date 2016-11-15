/******************************************************************************
* File Name          : sdadc_filter.h
* Date First Issued  : 03/09/2016
* Board              : F373 
* Description        : SDADC filtering of data from DMA buffer
*******************************************************************************/
/*


*/
#include <stdint.h>
#include "cic_filter_l_N2_M3_f3.h"
#include "cic_filter_ll_N2_M3_f3.h"
#include "sdadc_discovery.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDADC_FILTER
#define __SDADC_FILTER

#define LOUTSIZE	4	// Size of CIC output buffer

struct SDADC_FILTERED_BUFF
{
	struct CICLN2M3 cic;	// Filtering intermediate values
	struct CICLLN2M3 cicll;	// 
	long long lout[LOUTSIZE];	// Circular buffer
	long long *pin;		// Buffer input pointer
	long long *pout;	// Buffer output pointer
	uint8_t	flag;		// Flag for partriotic types
};

/******************************************************************************/
void sdadc1_filter_init(void);
/* @brief 	: Initialize for filtering SDADC 1
*******************************************************************************/
void sdadc2_filter_init(void);
/* @brief 	: Initialize for filtering SDADC 2
*******************************************************************************/
void sdadc3_filter_init(void);
/* @brief 	: Initialize for filtering SDADC 3
*******************************************************************************/
void sdadc123_filter_init(void);
/* @brief 	: Initialize for filtering SDADC 1,2,3
*******************************************************************************/
long long* sdadc_filter_get(uint16_t sdadcnum, uint16_t portseqnum);
/* @brief 	: Check for filtered data in SDADCx porty
 * @param	: sdadcnum = SDADC number (1,2,3)
 * @param	: partnum = port *sequence* number ( 0 - (NUMBERSDADCSx-1))
*******************************************************************************/

#endif

