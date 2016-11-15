/******************************************************************************
* File Name          : triac_ctl.c
* Date First Issued  : 08/07/2015
* Board              : f103
* Description        : Control for triac
*******************************************************************************/
/*

*/
#include "triac_ctl.h"
#include "tim_clk.h"
#include "tim2_vcal.h"
#include "pinconfig_all.h"
#include "vcal.h"

extern double tempcalib;	// Calibrated temperature
extern double therm[THERM_SHELL];

static double tempsetpt;		// Setpoint (target) temperature
static double error;	// Difference between temperature reading and setpt
static double error_prev;
static double integral;

static uint16_t state;
static uint16_t pwm;
/* 
EXT9 PA5 spi1-sclk -- Output pin to drive triac
   low pin = opto-iso FET off = triac off
*/
#define TRIAC_GAIN_P	10	// Proportion
#define TRIAC_GAIN_I	1	// Integral
#define TRIAC_GAIN_D	0.01	// Differential

#define OPTOBIT 5
//	if ((GPIO_ODR(GPIOA) & (1<<OPTOBIT)) == 0) // save
#define	TRIAC_ON (GPIO_BSRR(GPIOA) = (1<<OPTOBIT))	// Set bit = triac ON
#define	TRIAC_OFF (GPIO_BRR(GPIOA) = (1<<OPTOBIT))	// Reset bit = triac OFF
const struct PINCONFIGALL pin_TRIAC = {(volatile u32 *)GPIOA, OPTOBIT, OUT_PP, MHZ_50};

/******************************************************************************
 * void triac_ctl_init(void);
 * @brief	: Initialize 
*******************************************************************************/
void triac_ctl_init(void)
{
	pinconfig_all( (struct PINCONFIGALL *)&pin_TRIAC);
	TRIAC_OFF;
	state = 0;
	integral = 0;
	error_prev = 0;
	return;
}
/******************************************************************************
 * void triac_ctl_setpoint(double d);
 * @brief	: Set control loop setpoint
 * @param	: d = target temperature (deg C)
*******************************************************************************/
void triac_ctl_setpoint(double d)
{
	tempsetpt = d;	
	return;
}
/******************************************************************************
 * void triac_ctl_poll(void);
 * @brief	: Update triac control loop
*******************************************************************************/
void triac_ctl_poll(void)
{
	double sum;


	switch (state)
	{
	case 0:	// Off-idle 
		break;

	case 1: // Control loop
		
		error = tempsetpt - therm[THERM_SHELL];

		/* P of PID */
		sum = error * TRIAC_GAIN_P;
		
		/* I of PID */
		if ((pwm > 0) && (pwm < 64000))
		{ // Here, not in "wind-up/wind-down"
			integral += error;	// Accumulate error
		}
		sum += integral * TRIAC_GAIN_I;
		
		/* D of PID */
		// TODO

		/* Set next pwm period */
		if (sum < 0)
		{ // Off is the best we can do for cooling
			pwm = 0;
		}
		else
		{ // Set the pwn on/off time 
			pwm = sum;
		}
		tim2_vcal_setpwm(pwm);
	
		
		error_prev = error;			
		break;

	default:
		state = 0;
		break;
	}
	return;		

}

