/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : sensor_threshold.c
* Hackeroos          : deh
* Date First Issued  : 11/15/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for determining photocell thresholds
*******************************************************************************/
/*
The purpose is to build a histogram of the ADC values while the hapless op spins
the shaft.  From this we hope to divine the optimal threshold register settings
which will someday be saved for humanity in persistent storage.

Note: The EOC (End Of Conversion) flag gets reset when 'adcsensor_pod.c' gets an
adc watchdog interrupt as that routine reads the data register which automatically
resets the flag.  Since losing a a few out of many doesn't matter we just live 
with it.

******** Usage ******************

#define ADCHISTO_MAX	4000
#define NUMBERBINS	200
#define NUMBERDATAPTS	2000000
unsigned int bin[NUMBERBINS];
struct ADCHISTO strH = { NUBMERBINS, ADCHISTO_MAX, NUMBERDATAPTS, &bin[0] };


...
main()
...
... adc gets setup and is running, etc.
...
adc_histogram(&strH,(volatile u32 *)ADC1 );

*/

#include "sensor_threshold.h"
#include "libopenstm32/adc.h"
#include "libmiscstm32/printf.h"
#include "libusartstm32/usartallproto.h"




/*******************************************************************************
 * void adc_histogram(struct ADCHISTO *pH, volatile u32 * pADC);
 * @brief 	: Build histogram from ADC values
 * @param	: pH--pointer to struct with a bunch of things
 * @param	: ADC--ADC1, or ADC2
 * @return	: nothing to speak of
*******************************************************************************/
void adc_histogram(struct ADCHISTO *pH, u32 pADC)
{
	unsigned int x = 0;	// Count data points
	int dr;			// Data register retrieved
	int i;			// Index into array
	int w = pH->nMax / pH->nBins;	// Width of a bin

//printf("%d %d %u 0x%08x %d 0x%08x 0x%08x\n\r",pH->nBins,pH->nMax,pH->uiDp,(unsigned int)pH->p,w,(unsigned int)pADC,(unsigned int)ADC1);USART1_txint_send();
	while ( x++ < pH->uiDp )
	{
		while ((ADC_SR(pADC) & ADC_SR_EOC) == 0);	// Loop until End Of Conversion
		dr = ADC_DR(pADC);		// Data (which resets EOC flag)
		i = (dr / w);			// Data / Number adc counts per bin
		if (i >= pH->nBins) i = (pH->nBins -1);	// Stay within array boundary
		*(pH->p + i) += 1;		// Add a tick to this bin
	}
	return;
}
/*******************************************************************************
 * void adc_histogram_print(struct ADCHISTO *pH);
 * @brief 	: Printout the array
 * @param	: pH--pointer to struct with a bunch of things
 * @return	: nothing to speak of
*******************************************************************************/
void adc_histogram_print(struct ADCHISTO *pH)
{
	int w = pH->nMax / pH->nBins;	// Width of a bin
	int i;
	int val = 0;	

	for (i = 0; i < pH->nBins; i++)
	{
		printf("%4u%6u%10u\n\r",i,val,*(pH->p+i) ); USART1_txint_send();
		val += w;
	}
	return;
}

