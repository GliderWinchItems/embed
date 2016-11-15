/******************************************************************************
* File Name          : sdadc_recalib_stddev.c
* Date First Issued  : 03/30/2016
* Board              : F373 
* Description        : SDADC statistics on offset register
*******************************************************************************/
/* 
https://www.strchr.com/standard_deviation_in_one_pass
*/
#define NULL 0
#include "sdadc_recalib_stddev.h"

/******************************************************************************
 * void sdadc_recalib_stddev(struct STDDEVSDADC *p, int16_t reading);
 * @brief 	: Add new reading and recompute statistics
 * @param	: p = pointer to struct with intermediate values
 * @param	: reading = new data point
 * @return	: struct contains new computation
*******************************************************************************/
void sdadc_recalib_stddev(struct STDDEVSDADC *p, int16_t reading)
{
	p->sum += reading;
	p->sumsq = p->sumsq + (reading * reading);
	p->n += 1;		// Count data points
	p->mean = (float)p->sum/p->n;
	p->var = ((float)p->sumsq/p->n) - (p->mean * p->mean);
	return;
}
/******************************************************************************
 * void sdadc_recalib_stddev_init(struct STDDEVSDADC *p);
 * @brief 	: Reset for a new "run" of values
 * @param	: p = pointer to struct with intermediate values
*******************************************************************************/
void sdadc_recalib_stddev_init(struct STDDEVSDADC *p)
{
	p->n = 0;	// Counter
	p->sum = 0;
	p->sumsq = 0;
	return;
}
