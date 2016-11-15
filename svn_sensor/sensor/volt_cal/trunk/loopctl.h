/******************************************************************************
* File Name          : loopctl.h
* Date First Issued  : 09/29/2015
* Board              : f103
* Description        : Control loop for voltage source
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOOPCTLVCAL
#define __LOOPCTLVCAL

#include <stdint.h>
#include "vcal_idx_v_struct.h"

/* Used to return variables of interest for monitoring if it "works" */
struct LOOPCTL_STATE
{
	int8_t	state;		// State machine
	double setpoint;	// Current setpoint
	uint32_t secs;		// Running seconds from DWT register
	uint32_t ipwm;		// PWM setting
	float therm_ctl;	// Current therm temp
	float looperr;		// Current loop error * dP
	uint32_t secsref;	// Measuring current time duration
	float derivative;	// Derivative of loop error * dD
	float integral;		// Integral * dI
	double proportional;
};

/*******************************************************************************/
int loopctl_setstate(uint8_t s);
/* @brief	: Set state (edit for valid codes)
 * @param	: s: 0 = Idle; 
 *		:    1 = Pasteurize;
 *		:   11 = Ferment; 
 * @return	: 0 = OK; not zero = illegal state number
*******************************************************************************/
struct LOOPCTL_STATE* loopctl_poll(double therm, struct VSOVEN* poven);
/* @brief	: Update oven PWM
 * @param	: therm = temperature reading (deg C) of control pt
 * @param	: poven = pointer to oven struct VSOVEN
 * @return	: pointer to loop values struct
*******************************************************************************/
void loopctl_init_oven(struct VSOVEN* p);
/* @brief	: Init oven parameters
 * @param	: p = pointer to oven struct VSOVEN
*******************************************************************************/
void loopctl_triptimeout(void);
/* @return	: Cause timeout to expire
*******************************************************************************/


#endif 
