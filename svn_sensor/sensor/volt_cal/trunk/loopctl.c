/******************************************************************************
* File Name          : loopctl.c
* Date First Issued  : 09/29/2015
* Board              : f103
* Description        : Control loop for voltage source
*******************************************************************************/
/*
This timer runs the PWM for the fan voltage (speed) control. 
Frame rate 100 usec.

Routine does not use interrupts.

PB8 CH3 cooling fan control
*/
#include <stdio.h>
#include "loopctl.h"
#include "vcal_idx_v_struct.h"

#include "libopenstm32/gpio.h"
#include "libopenstm32/timer.h"
#include "libmiscstm32/DTW_counter.h"

#include "common_can.h"
#include "pinconfig_all.h"
#include "printf.h"
#include "tim2_vcal.h"
//#include "OlimexLED.h"
#include "derivative_iir.h"

#include "fpprint.h"
#include "printf.h"
#include "libusartstm32/usartallproto.h"

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

#define ONESECINC (64000000/1)	// One second tick duration
static uint32_t ticks_1sec;
static uint32_t secs;		// Running seconds from DWT register

void 	(*loop_ctl_ptr)(void) = NULL;	// Address of function to call at timer complete tick
void 	(*loop_ctl_ll_ptr)(void) = NULL;	// Low Level function call

static int8_t	state = 0;	// State machine
#define INIT_DELAY	4	// Number of secs for readings to stabilize


static uint32_t ipwm;

/* Integrator init, given Shell temp and ambient temp */
#define INTEGRATEINIT_A -4000	// Coefficient for a + b*x
#define INTEGRATEINIT_B   434	// Coefficient for a + b*x

/* Full ON heat-to-temp cutoff temp. */
#define STOREDHEATCONSTANT_KM_P  0.0100	// Formula constant for stored heat
#define STOREDHEATCONSTANT_KM_F  0.0200	// Formula constant for stored heat
#define STABILIZETIMEDELAY_P    200	// Pasteur: Time delay for system temp to stablize (secs)
#define STABILIZETIMEDELAY_F   1070	// Ferment: Time delay for system temp to stablize (secs)
#define AMBIENT_TEMP  	      78.0	// Ambient temp (deg F)

/* Current values of control loop */
static struct LOOPCTL_STATE loopvals;

/******************************************************************************
 * void loopctl_init_oven(struct VSOVEN* p);
 * @brief	: Init oven parameters
 * @param	: p = pointer to oven struct VSOVEN
*******************************************************************************/
void loopctl_init_oven(struct VSOVEN* p)
{
	/* Will cause iir filter to initialize. */
	p->iir.sw = 0;	

	/* Init next seconds tick count. */
	ticks_1sec = DTWTIME + ONESECINC;
	return;
}
/******************************************************************************
 * static double loop(double therm, struct VSOVEN* p);
 * @brief	: Update PWM
 * @param	: therm = oven temperature (degC)
 * @param	: p = pointer to oven struct VSOVEN
 * @return	: looperr
*******************************************************************************/
static double looperr;
static 	double derivative;

static double loop(double therm, struct VSOVEN* p)
{
	double dtmp;

	double integrator_tmp;
	double looperrP;

	looperr = (p->setpt - therm);
	loopvals.looperr = p->setpt;

	/* 'I' of PID */
	integrator_tmp = p->integral + (looperr * p->i);

	/* 'D' of PID */
	// Control derivative 'D' of PID
	dtmp = iir_1(therm, &p->iir);
	derivative = (dtmp - p->deriv_prev) * p->d;
	p->deriv_prev = therm;

	/* 'P' of PID */
	looperrP = looperr * p->p;

	dtmp = looperrP + integrator_tmp + derivative;

	/* Set pwm, but cognizant of the limits. */
	if ((dtmp < 64000.0) && (dtmp > 0))
	{ // (P+I+D) is NOT beyond limits
		p->integral = integrator_tmp; // Update saved integral
		ipwm = dtmp;
	}
	else
	{ // Here, either less than zero or over max
		if (dtmp < 0)
		{ // Less than zero	
			ipwm = 0;
		}
		else
		{ // Over max
			ipwm = 64001;
		}
	}
	tim2_vcal_setpwm(ipwm);	// Set timer pwm

	return looperr;	// Return looperr (deg C error)
}
/******************************************************************************
 * int loopctl_setstate(uint8_t s);
 * @brief	: Set state (edit for valid codes)
 * @param	: s: 0 = Idle; 
 *		:    1 = Pasteurize;
 *		:   11 = Ferment; 
 * @return	: 0 = OK; not zero = illegal state number
*******************************************************************************/
int loopctl_setstate(uint8_t s)
{

	state = 0;
	return s;
}

/******************************************************************************
 * struct LOOPCTL_STATE* loopctl_poll(double therm, struct VSOVEN* poven);
 * @brief	: Update oven PWM
 * @param	: therm = temperature reading (deg C) of control pt
 * @param	: poven = pointer to oven struct VSOVEN
 * @return	: pointer to loop values struct
*******************************************************************************/
struct LOOPCTL_STATE* loopctl_poll(double therm_ctl, struct VSOVEN* poven)
{
	float fret = 0;	// Float return value

	/* Time duration: 1 sec ticks */
	if ( ((int)ticks_1sec - (int)DTWTIME) < 0) 
	{
		ticks_1sec += ONESECINC;
		secs += 1;
	}
	switch (state)
	{
	case 0: // OTO delay to allow readings to stablize
		if (secs <= INIT_DELAY) break;
		state = 1;

	case 1: 
		fret = loop(therm_ctl, poven); 
		break;
	default:
		printf("#### EGADS! Bad case number: %d\n\r", state);
		state = 0;
	}
	/* Return a pointer to some stuff to entertain the hapless Op. */
	loopvals.state = state;
	loopvals.looperr = fret;
	loopvals.ipwm = ipwm;
	loopvals.secs = secs;
	loopvals.therm_ctl = therm_ctl;
	loopvals.setpoint = poven->setpt;
	loopvals.integral = poven->integral;
	loopvals.proportional = looperr * poven->p;
	loopvals.derivative = derivative;

	return &loopvals;
	
}
