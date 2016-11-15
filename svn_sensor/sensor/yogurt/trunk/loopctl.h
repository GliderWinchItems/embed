/******************************************************************************
* File Name          : loopctl.h
* Date First Issued  : 08/17/2015
* Board              : f103
* Description        : Control loop for yogurt maker
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOOPCTL
#define __LOOPCTL

#include <stdint.h>

/* Used to return variables of interest for monitoring if it "works" */
struct LOOPCTL_STATE
{
	uint32_t duration;	// Time duration at temp (secs)
	int8_t	state;		// State machine
	double setpoint;	// Current setpoint
	uint32_t secs;		// Running seconds from DWT register
	uint32_t secs0;		// End-point seconds};
	uint32_t ipwm;		// PWM setting
	float therm_ctl;	// Current therm temp
	float looperr;		// Current loop error * dP
	uint32_t secsref;	// Measuring current time duration
	float derivative;	// Derivative of loop error * dD
	float integral;		// Integral * dI
	float derivative_a;
};

/******************************************************************************/
void loopctl_setfan(uint32_t x);
/* @brief	: Set fan 
 * @param	: x: 0 = OFF
 *		:    64001 = FULL ON
 *		: (0 < x < 64000) = PWM (x / 64000)
*******************************************************************************/
int loopctl_setstate(uint8_t s);
/* @brief	: Set state (edit for valid codes)
 * @param	: s: 0 = Idle; 
 *		:    1 = Pasteurize;
 *		:   11 = Ferment; 
 * @return	: 0 = OK; not zero = illegal state number
*******************************************************************************/
int8_t loopctl_getstate(void);
/* @return	: current value of 'state'
*******************************************************************************/
struct LOOPCTL_STATE* loopctl_poll(double therm_ctl, double therm_shell, double therm_airin);
/* @brief	: Update power PWM and fan
 * @param	: therm_ctl = temperature reading (deg F) of control pt
 * @param	: therm_shell = temperature reading (deg F) of shell
 * @param	: therm_airin = ambient temperature (deg F)
 * @return	: pointer to loop values struct
*******************************************************************************/
void loopctl_init(double therm_airin);
/* @brief	: Init
 * @param	: therm_airin = ambient temperature (deg F)
*******************************************************************************/
void loopctl_triptimeout(void);
/* @return	: Cause timeout to expire
*******************************************************************************/


#endif 
