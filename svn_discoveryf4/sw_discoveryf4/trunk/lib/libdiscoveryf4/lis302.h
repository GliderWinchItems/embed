/*****************************************************************************
* File Name          : lis302.h
* Date First Issued  : 01/21/2015
* Board              : Discovery F4
* Description        : ST LIS302 accelerometer operation
*******************************************************************************/

#ifndef __LIS302
#define __LIS302

#include <stdint.h>

#define LISAVECT	100	// Count in averaging readings

struct LIS302AVE
{
	uint32_t ct;
	int32_t x;
	int32_t y;
	int32_t z;
};

/******************************************************************************/
void lis302_init(void);
/*  @brief	: Initialize the mess
*******************************************************************************/ 
struct LIS302AVE *lis302_get(void);
/* @brief	: Get xyz data if ready.
 * @return	: NULL = no data, or ptr to data
*******************************************************************************/ 

extern void 	(*timer_sw_ptr2)(void);

#endif

