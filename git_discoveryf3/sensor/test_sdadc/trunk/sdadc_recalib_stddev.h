/******************************************************************************
* File Name          : sdadc_recalib_stddev.h
* Date First Issued  : 03/30/2016
* Board              : F373 
* Description        : SDADC statistics on offset register
*******************************************************************************/
/*
https://www.strchr.com/standard_deviation_in_one_pass
*/
#include <stdint.h>

/* Defined to prevent confusion and cursing. */
#ifndef __SDADC_RECALIB_STDDEV
#define __SDADC_RECALIB_STDDEV

struct STDDEVSDADC
{
	int64_t sumsq;	// sum squared
	int32_t sum;	// sum
	uint32_t n;	// Number of data pts in computation
	float mean;	// Computed from foregoing
	float var;	// Computed from foregoing
};

/******************************************************************************/
void sdadc_recalib_stddev(struct STDDEVSDADC *p, int16_t reading);
/* @brief 	: Add new reading and recompute statistics
 * @param	: p = pointer to struct with intermediate values
 * @param	: reading = new data point
 * @return	: struct contains new computation
*******************************************************************************/
void sdadc_recalib_stddev_init(struct STDDEVSDADC *p);
/* @brief 	: Reset for a new "run" of values
 * @param	: p = pointer to struct with intermediate values
*******************************************************************************/

#endif

