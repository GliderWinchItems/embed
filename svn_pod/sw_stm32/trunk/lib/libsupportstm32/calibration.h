/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : calibration.h
* Hackeroos          : deh
* Date First Issued  : 07/17/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Calibration of various devices for POD
*******************************************************************************/
/* NOTE:
The struct is for calibration constants.  'calibration.c' has the default values
which are defined below.  Later, we may add a table based on this struct which
has values specific to the particular board.

Additions to this should be made at the bottom of the struct since a block may
been previously stored on the SD Card.
*/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_pod/trunk/pod_v1/tickadjust.c
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CALIBRATION
#define __CALIBRATION

/* ADC calibrations ---
  Calibration value = ( (measured voltage) / (adc reading) ) * 65536 * 1000, rounded 
  ADC regulator voltage: 3.20v = 4095 ADC reading 
*/
#define ADCMAXREAD	4095				// Maximum ADC reading
#define	ADCREGVOLTS	3200				// Regulator voltage (mv)
#define ADCMVPERSTEP	((65536*ADCREGVOLTS)/ADCMAXREAD) 	// Millivolts per ADC reading step
#define TOPCELLRATIO	((100*(10+18)/10))			// 18K|10K divider: top battery cell (6v - 8.4v)
#define	BOTCELLRATIO	((100*(10+5)/10))			// 10K| 5K divider: bottom battery cell (3v - 4.2v)
#define TOPCELLMVNOM	(TOPCELLRATIO*ADCMVPERSTEP)/100	// Nominal: Top cell mv per adc step
#define BOTCELLMVNOM	(BOTCELLRATIO*ADCMVPERSTEP)/100	// Nominal: Bottom cell mv per adc step
#define TOPCELLCALIBRATED (1000*TOPCELLMVNOM)/1000	// Adjustment for measured calibration	
#define BOTCELLCALIBRATED (1000*BOTCELLMVNOM)/1000	// Adjustment for measured calibration 			

#define CALVERSION	0	// Calibration struct version number

struct CALBLOCK
{
	unsigned long		version;	// Calibration version number
	unsigned long		adcbot;		// Top cell calibration * adc reading -> actual voltage (mv)
	unsigned long		adctop;		// Bot cell calibration * adc reading -> actual voltage (mv)
	unsigned long		celllow;	// Below this cell voltage shutdown (mv)
	signed long		adcppm;		// ADC offset for temp compensation table lookup of 32 KHz osc (see also '../devices/adcppm.c')
	signed long		tmpoff;		// Temp offset conversion table lookup (see also '../devices/adcthermtmp.c')
	signed long		ppmoff;		// ppm * 100 offset for 32 KHz osc nominal freq
	signed long		accel_offset[3];// ADC value for zero reading
	signed long		accel_scale[3];	// Scaling for g * 100
	signed long		load_cell;	// Convert readings: counts per kg*10
	signed long		load_cell_zero;	// Trim up the zero reading
	signed long		xtal_To;	// Turnpoint temperature for 32 KHz xtal (deg C * 100)
	signed long		xtal_alpha;	// 32 KHz xtal temp coefficient (ppm = alpa * (T -To)^2)
	signed long		xtal_a;		// Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3) (see tickadjust.c)
	signed long		xtal_b;		// Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3)
	signed long		xtal_c;		// Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3)
	signed long		xtal_d;		// Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3)
	signed long		xtal_t;		// Adjustment to temperature that is input to polynomial 
	signed long		xtal_o;		// Adjustment of offset after polynomial computation 32 KHz (see 'void RTC_tickadjust_poly_compute(void)' in @1)
	signed long		xtal_o8;	// Adjustment of offset after polynomial computation  8 KHz (see 'void RTC_tickadjust_poly_compute(void)' in @1)
	signed long	      spare[39];	// Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 ***
	unsigned long		crc;		// Last item is crc

};

/******************************************************************************/
void calibration_init_default(struct CALBLOCK *p);
/* @brief	: Initialize calibration block
 * @param	: Pointer to the calibrationblock
 ******************************************************************************/

/* Default calibrations for various devices (@15) */
extern struct CALBLOCK strDefaultCalib;	 // Default calibration


#endif
