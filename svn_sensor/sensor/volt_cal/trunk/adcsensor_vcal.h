/******************************************************************************
* File Name          : adcsensor_vcal.h
* Date First Issued  : 08/04/2015
* Board              : Olimex
* Description        : ADC routines for f103 Olimex
*******************************************************************************/
/*


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCSENSOR_YOGURT
#define __ADCSENSOR_YOGURT

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


#define WAITDTW(tick)	while (( (int)tick ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 )

#define DMA1_CH1_PRIORITY	0x60	// * ### Lower than SYSTICK (ADC filtering after DMA interrupt)
#define NVIC_I2C2_EV_IRQ_PRIORITY 		0xb0	//

#define NUMBERADCCHANNELS_TEN	6	// Number of channels to scan
#define NUMBERSEQUENCES		16	// Number of sequences in 1/2 of the buffer
#define DECIMATION_TEN		16	// Decimation ratio
#define DISCARD_TEN		32	// Number of readings to discard before filtering starts
#define CICSCALE		13	// Right shift count to scale result

/* ADC usage
Usage for Voltage calibrator (POD) board
PA3  - IN3 AD7799 temperature
PC 0 - IN10 Voltage source temperature
     - Internal temperature reference
     - Internal voltage source
PC1  - 2.5v precision voltage source
PC2  - Direct measurement pin
*/
/* DMA double buffer ADC readings. */
struct ADCDR_TENSION
{
	uint32_t in[2][NUMBERSEQUENCES][NUMBERADCCHANNELS_TEN];	// ADC_DR is stored for each channel in the regular sequence
	unsigned int cnt;		// DMA interrupt counter
	unsigned short flg;		// Index of buffer that is not busy (0, or 1)
};

/******************************************************************************/
void adcsensor_vcal_sequence(void);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/


/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
// These hold the address of the function that will be called
extern void 	(*dma_ll_ptr)(void);		// DMA -> FSMC  (low priority)

extern uint32_t	adc_readings_cic[2][NUMBERADCCHANNELS_TEN];	// Last computed & doubly filtered value for each channel

extern volatile int32_t	adc_temp_flag[NUMBERADCCHANNELS_TEN];	// Signal main new filtered reading ready

extern unsigned int cicdebug0,cicdebug1;

#endif 
