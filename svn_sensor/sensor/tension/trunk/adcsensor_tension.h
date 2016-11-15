/******************************************************************************
* File Name          : adcsensor_tension.h
* Date First Issued  : 02/14/2015
* Board              : POD
* Description        : ADC routines for f103 pod board with two AD7799
*******************************************************************************/
/*


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCSENSOR_TENSION
#define __ADCSENSOR_TENSION

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


#define WAITDTW(tick)	while (( (int)tick ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 )

//$ #define ADC_FSMC_PRIORITY	0x80	//    
#define ADC_TIM5_PRIORITY	0x80

#define DMA1_CH1_PRIORITY	0x60	// * ### Lower than SYSTICK (ADC filtering after DMA interrupt)

#define NUMBERADCTHERMISTER_TEN	4	// Number of Thermistor channels
#define NUMBERADCCHANNELS_TEN	4	// Number of channels to scan
#define NUMBERSEQUENCES		16	// Number of sequences in 1/2 of the buffer
#define DECIMATION_TEN		16	// Decimation ratio
#define DISCARD_TEN		8	// Number of readings to discard before filtering starts
#define CICSCALE		12	// Right shift count to scale result

/* ADC usage
PA 3 ADC123-IN3	Thermistor on 32 KHz xtal..Thermistor: AD7799 #2
PB 0 ADC12 -IN8	Bottom cell of battery.....Not scanned
PB 1 ADC12 -IN9	Top cell of battery........Not scanned
PC 0 ADC12 -IN10	Accelerometer X....Thermistor: load-cell #1
PC 1 ADC12 -IN11	Accelerometer Y....Thermistor: load-cell #2
PC 2 ADC12 -IN12	Accelerometer Z....Thermistor: AD7799 #1
PC 3 ADC12 -IN13	Op-amp.............Not scanned
-- - ADC1  -IN16	                   Internal temp ref
-- - ADC1  -IN17	                   Internal voltage ref (Vrefint)

*/

/* DMA double buffer ADC readings. */
struct ADCDR_TENSION
{
	uint32_t in[2][NUMBERSEQUENCES][NUMBERADCCHANNELS_TEN];	// ADC_DR is stored for each channel in the regular sequence
	unsigned int cnt;		// DMA interrupt counter
	unsigned short flg;		// Index of buffer that is not busy (0, or 1)
};

/******************************************************************************/
void adcsensor_tension_sequence(void);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/


/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
// These hold the address of the function that will be called
extern void 	(*dma_ll_ptr)(void);		// DMA -> FSMC  (low priority)

extern uint32_t	adc_readings_cic[2][NUMBERADCCHANNELS_TEN];	// Last computed & doubly filtered value for each channel

extern int32_t	adc_temp_flag[NUMBERADCCHANNELS_TEN];	// Signal main new filtered reading ready

extern unsigned int cicdebug0,cicdebug1;

#endif 
