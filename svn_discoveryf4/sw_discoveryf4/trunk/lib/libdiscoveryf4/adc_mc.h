/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : adc_mc.h
* Date First Issued  : 12/22/2013
* Board              : Discovery F4
* Description        : ADC routines for Master Controller
*******************************************************************************/
/*


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_MC
#define __ADC_MC

#define NUMBERADCCHANNELS_MC	5	// Number of channels for a regular sequence 
#define NUMBERSEQUENCES		16	// Number of sequences in 1/2 of the buffer
#define DECIMATION_MC		32	// Decimation ratio
#define DISCARD_MC		32	// Number of readings to discard before filtering starts
#define CICSCALE		18	// Right shift count to scale result

/* ADC usage
PA0 ADC123-IN0	
PA1 ADC123-IN1  
PA3 ADC123-IN2	

Revision for MC
PC1 ADC123_IN11 Control Lever
PC2 ADC123_IN12
PC4 ADC12 -IN14
*/

/* The following is a double buffer used with the DMA */
struct ADCDR_ENG
{
	// int in [Double buffer][number sequences in one buffer][number adc's in one sequence]
	short in[2][NUMBERSEQUENCES][NUMBERADCCHANNELS_MC]; 
	unsigned int cnt;		// DMA interrupt counter
	unsigned short flg;		// Index of buffer that is not busy (0, or 1)
};

/******************************************************************************/
int adc_mc_init_sequence(void);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/

/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
// These hold the address of the function that will be called
extern void 	(*dma_ll_ptr)(void);		// DMA -> FSMC  (low priority)
extern long	adc_last_filtered[NUMBERADCCHANNELS_MC];	// Last computed value for each channel


#endif 
