/******************************************************************************
* File Name          : loopctl.c
* Date First Issued  : 08/17/2015
* Board              : f103
* Description        : Control loop for yogurt maker
*******************************************************************************/
/*
This timer runs the PWM for the fan voltage (speed) control. 
Frame rate 100 usec.

Routine does not use interrupts.

PB8 CH3 cooling fan control
*/
#include <stdio.h>
#include "loopctl.h"
#include "yogurt_idx_v_struct.h"

#include "libopenstm32/gpio.h"
#include "libopenstm32/timer.h"
#include "libmiscstm32/DTW_counter.h"

#include "common_can.h"
#include "pinconfig_all.h"
#include "printf.h"
#include "tim3_yog.h"
#include "tim4_yog.h"
#include "OlimexLED.h"
#include "yog_derivative.h"
#include "derivative_ave.h"
#include "derivative_oneside.h"
#include "derivative_iir.h"

#include "fpprint.h"
#include "printf.h"
#include "libusartstm32/usartallproto.h"

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

#define ONESECINC (64000000/1)	// One second tick duration
static uint32_t ticks_1sec;
static uint32_t secs;		// Running seconds from DWT register
static uint32_t secs0;		// Seconds count "at temperature"
static uint32_t secsref;	// Measure secs for current phase
static uint32_t secsd;		// Seconds count of "temperature stabilization"
static uint32_t secs1;		// Seconds of duration "at temperature:"
static int sw_p;		// Pasteur completed switch (used by Ferment phase)

void 	(*loop_ctl_ptr)(void) = NULL;	// Address of function to call at timer complete tick
void 	(*loop_ctl_ll_ptr)(void) = NULL;	// Low Level function call

static uint32_t duration;	// Time duration at temp (secs)
static struct HEATCOOL* phtcl = NULL;

static double setpoint;		// set-point in deg C

static double integrator_tmp = 5000; 
static double integrator = 5000;
static double derivative;
static double looperrP;
static double derivative_a;

static int8_t	state = -1;	// State machine
#define INIT_DELAY	4	// Number of secs for readings to stabilize

// Control law parameters as doubles (i.e. F103)
static double dP;
static double dI;
static double dD;

static uint32_t ipwm;
#define IIR_RC1 	50.0
static struct DELTAIIR iir1 = { \
IIR_RC1,
0.0,
0.0,
0.0,
0.0,
0,
};
#define IIR_RC2 	50.0
static struct DELTAIIR iir2 = { \
IIR_RC2,
0.0,
0.0,
0.0,
0.0,
0,
};

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
 * void loopctl_init(double therm_airin);
 * @brief	: Init
 * @param	: therm_airin = ambient temperature (deg F)
*******************************************************************************/
void loopctl_init(double therm_airin)
{
	// OTO init of coefficients: doubles from floats
	dP = thm.p;	// Setup coefficients as doubles from floats
	dI = thm.i;
	dD = thm.d;

	/* Estimate integrator value at setpoint, given ambient temp. */
	integrator_tmp = INTEGRATEINIT_A + INTEGRATEINIT_B * (setpoint - therm_airin);
	integrator = integrator_tmp;

	/* Reset derivative(s) history. */
	derivative_iir_reset(&iir1);
	derivative_iir_reset(&iir2);

	/* Init next seconds tick count. */
	ticks_1sec = DTWTIME + ONESECINC;
	return;
}
/******************************************************************************
 * static double loop(double therm, double thm_shell, int on);
 * @brief	: Update PWM
 * @param	: therm = pot temperature (deg F)
 * @param	: thm_shell = shell temp (deg F)
 * @param	: on = +1 = full ON; -1 = full OFF; 0 = computed control
 * @return	: looperr
*******************************************************************************/
static double looperr;
static double loop(double therm, double thm_shell, int on)
{
	looperr = (setpoint - therm);

	/* 'I' of PID */
	integrator_tmp = integrator + (looperr * dI);

	/* Derivatives */
	// Control derivative 'D' of PID
	derivative   = derivative_iir(&iir1, looperr) * dD;
	// Shell derivative
	derivative_a   = derivative_iir(&iir2, thm_shell);

	/* 'P' of PID */
	looperrP = looperr * dP;

	double dtmp = looperrP + integrator_tmp + derivative;

	/* Set pwm, but cognizant of the limits. */
	if ((dtmp < 64000.0) && (dtmp > 0))
	{ // (P+I+D) is NOT beyond limits
		integrator = integrator_tmp;
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
	/* Override PWM value in some cases. */
	if (on == -1)
		ipwm = 0;	// Full OFF
	if (on ==  1)
		ipwm = 64001;	// Full ON

	tim3_yog_setpwm(ipwm);	// Set timer pwm

	return looperr;	// Return looperr (deg C error)
}
/******************************************************************************
 * void loopctl_setfan(uint32_t x);
 * @brief	: Set fan 
 * @param	: x: 0 = OFF
 *		:    64001 = FULL ON
 *		: (0 < x < 64000) = PWM (x / 64000)
*******************************************************************************/
void loopctl_setfan(uint32_t x)
{
	tim4_yog_setpwm(x);	// Set PWM tick count
	return;
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
	int i = 0;
	switch (s)
	{
	case   0: state = s; break;	// Idle
	case   1: state = s; break;	// Start Pasteur
	case  11: state = s; break;	// Start Ferment
	case  31: state = s; break;	// Swtich to cool down
	case  39: state = s; break;	// Swtich to cool down
	case  122: state = 11; 
		sw_p = 1;
		break;	// Start Ferment, skip stabilize
	default: i = -1; break;
	}
	return i;
}
/******************************************************************************
 * int8_t loopctl_getstate(void);
 * @return	: current value of 'state'
*******************************************************************************/
int8_t loopctl_getstate(void)
{
	return state;
}
/******************************************************************************
 * void loopctl_triptimeout(void);
 * @return	: Cause timeout to expire
*******************************************************************************/
void loopctl_triptimeout(void)
{
	secs0 = secs;
	return;
}

static uint32_t secs_prev;
static uint32_t tics(void)
{
	if (secs != secs_prev)
	{
		secs_prev = secs;	
		return 1;
	}
	return 0;
}
static uint32_t secs_prev1;
static uint32_t tics1(void)
{
	if (secs != secs_prev1)
	{
		secs_prev1 = secs;	
		return 1;
	}
	return 0;
}

/******************************************************************************
 * struct LOOPCTL_STATE* loopctl_poll(double therm_ctl, double therm_shell, double therm_airin);
 * @brief	: Update power PWM and fan
 * @param	: therm_ctl = temperature reading (deg F) of control pt
 * @param	: therm_shell = temperature reading (deg F) of shell
 * @param	: therm_airin = ambient temperature (deg F)
 * @return	: pointer to loop values struct
*******************************************************************************/
struct LOOPCTL_STATE* loopctl_poll(double therm_ctl, double therm_shell, double therm_airin)
{
char s[32];
	float fret = 0;	// Float return value
	double delta_t2;
/* ### OVERRIDE THERMISTER TEMP ### */
therm_airin = AMBIENT_TEMP;

	/* Time duration: 1 sec ticks */
	if ( ((int)ticks_1sec - (int)DTWTIME) < 0) 
	{
		ticks_1sec += ONESECINC;
		secs += 1;
	}
	switch (state)
	{
	case -1: // OTO delay to allow readings to stablize
		loopctl_setfan(0);	// Fan OFF
		if (secs <= INIT_DELAY) break;
		secsref = secs;		// Measure time of next phase
		state = 0;

	case 0: // Idle
		setpoint = 0.0;
		loopctl_init(therm_airin);		// Reset derivative history
		integrator = 0;
		fret = loop(therm_ctl, therm_shell,  -1); 	// Update control loop, OFF
		loopctl_setfan(0);		// Fan OFF
		OlimexLED_settogglerate(3);	// Slow flash
		break;
	/* ============ Pasteurization phase ================================== */

	case 1:	// Init for Pasteurization phase
		/* Delay start if 'delay' count is not zero. */
		if (thm.delay > 0)
		{
			if (secsd != secs)
			{
				secsd = secs;
				thm.delay -= 1;
			}
			break;
		}
		/* Start Pasteur heating phase. */
		loopctl_init(therm_airin);		// Reset derivative history
		duration = thm.htcl[0].dur * 3600; 	// Duration at temp (in secs)
		setpoint = thm.htcl[0].heat;		// Set temp (deg C)
		phtcl = &thm.htcl[0];			// Ptr to heat/dur/cool parameters
		sw_p = 0;				// Completion switch
		secsref = secs;				// Measure time of next phase
		secs0 = 0;				// Count secs "at temp"
		secs1 = (thm.htcl[0].dur * 3600); 	// Duration "at temperature" (secs)
		secsd = 0; 				// Stabilization secs
		state = 2;

	case 2: // Full ON until full OFF and stabilize time = estimated setpoint temp
		OlimexLED_settogglerate(20);	// 0.5 sec ON, 0.5 sec OFF flash
		fret = loop(therm_ctl, therm_shell, 1); 	// Update control loop, full ON
		loopctl_setfan(0);		// Fan OFF

		/* Count time pot is near setpoint temp. */
		if (fret < 2.0) secs0 += tics(); // Count secs near setpt temperature

		/* Compute control temp to set heater from full ON to full OFF */
		delta_t2 = (therm_shell - setpoint) * STOREDHEATCONSTANT_KM_P;
fpformat(s, delta_t2); printf("%s ",s);
fpformat(s, fret); printf("%s \n\r",s);
		if (fret > delta_t2) break;
		state = 21;
		break;

	case 21: // Waiting for temp to stabilize
		fret = loop(therm_ctl, therm_shell, -1); 	// Update control loop, full OFF
		if (fret < 2.0) secs0 += tics(); // Count secs near setpt temperature
		if (((int)secs0 - (int)secs1) > 0)// Time-at-temp expired? 
			{state = 31; break;}	// Skip to init for cool down
		secsd += tics1();		// Count stabilizing secs
printf("S: %d %d\n\r",secsd, STABILIZETIMEDELAY_P);
		if ( ((int)secsd - (int)STABILIZETIMEDELAY_P) < 0 ) break;

		// Here, we expect the temp to be close to the setpoint
		secsref = secs;		// Measure time of next phase
		state = 3;

	case 3: // Time at temperature
		fret = loop(therm_ctl, therm_shell,  0);
		OlimexLED_settogglerate(10);	// 1 sec ON, 1 sec OFF flash
		if (fret < 2.0) secs0 += tics(); // Count secs near setpt temperature
		if (((int)secs0 - (int)secs1) < 0) break;
	case 31:
		// Here, time duration "at temperature" has expired
		loopctl_setfan(64001);		// Fan ON
		// Set cool-down target temp (deg C)
		setpoint = thm.htcl[0].cool;	// New setpoint (e.g. 110 F)
		loopctl_init(therm_airin);	// Re-init control loop
		secsref = secs;			// Measure time of next phase
		state = 4;
	
	case 4: // Cool down from Pasteur phase
		fret = loop(therm_ctl, therm_shell,  0); 	// Update control loop, control law
		OlimexLED_settogglerate(40);	// 1 sec ON, 1 sec OFF flashc
		if ( fret < 0.0 ) break;	
		// Here we are within 1 deg of setpoint, i.e. close enough
		loopctl_setfan(0);		// Fan OFF
		loopctl_init(therm_airin);	// Re-init control loop
		state = 5;			// Idle until hapless Op presses a button 
		sw_p = 1;	// Show the ended Pasteur phase.
		break;

	case 5: 
		OlimexLED_settogglerate(80);	// Fast flash
		fret = loop(therm_ctl, therm_shell,  0); 	// Update control loopbreak;
		secsref = secs;			// Measure time of next phase
		break;

	/* ======== Ferment phase ================================================ */
	case 11: // Init for Ferment phase
		duration = thm.htcl[1].dur * 3600; // Duration at temp (in secs)
		setpoint = thm.htcl[1].heat;	// Set temp (deg C)
		loopctl_init(therm_airin);		// Reset derivative history
		phtcl = &thm.htcl[1];		// Ptr to heat/dur/cool parameters
		secsref = secs;			// Measure time of next phase
		secs0 = 0;			// Count secs "at temp"
		secs1 = (thm.htcl[1].dur * 3600);// Duration "at temperature" (secs)
		secsd = 0;			// Stabilization secs
		state = 12;

	case 12: // Heating to setpoint temperature
		OlimexLED_settogglerate(20);	// 0.5 sec ON, 0.5 sec OFF flash
		fret = loop(therm_ctl, therm_shell, 1); 	// Update control loop, full ON
		loopctl_setfan(0);		// Fan OFF

		/* Count time pot is near setpoint temp. */
		if (fret < 2.0) secs0 += tics(); // Count secs near setpt temperature

		/* Compute control temp to set heater from full ON to full OFF */
		delta_t2 = (therm_shell - setpoint) * STOREDHEATCONSTANT_KM_F;
fpformat(s, delta_t2); printf("D: %s ",s);
fpformat(s, fret); printf("%s \n\r",s);
		if (fret > delta_t2) break;
		if (sw_p != 0)
		{ // Here, Pasteur completed so temp should be at Ferment setpt
			sw_p = 0;	// JIC reset switch
			state = 13;	// Skip stabilize temp.
			break;
		}
		state = 121;

	case 121: // Waiting for temp to stabilize
		fret = loop(therm_ctl, therm_shell, -1); 	// Update control loop, full OFF
		if (fret < 2.0) secs0 += tics(); // Count secs near setpt temperature
		if (((int)secs0 - (int)secs1) > 0)// Time-at-temp expired? 
			{state = 39; break;}	// Skip to init for cool down
		secsd += tics1();		// Count stabilizing secs
printf("S: %d %d\n\r",secsd, STABILIZETIMEDELAY_F);
		if ( ((int)secsd - (int)STABILIZETIMEDELAY_F) < 0 ) break;

		// Here, we expect the temp to be close to the setpoint
		phtcl = &thm.htcl[1];		// Ptr to heat/dur/cool parameters
		loopctl_init(therm_airin);	// Re-init control loop
		secsref = secs;		// Measure time of next phase
		state = 13;

	case 13: // Time at temperature

		fret = loop(therm_ctl, therm_shell,  0);	// Update control loop, control law
		OlimexLED_settogglerate(10);	// 1 sec ON, 1 sec OFF flash
		if (fret < 2.0) secs0 += tics(); // Count secs near setpt temperature
		if (((int)secs0 - (int)secs1) < 0) break;
	case 39:
		// Here, time duration "at temperature" has expired
		loopctl_setfan(64001);	// Fan ON
		// Set cool-down target temp (deg C)
		setpoint = thm.htcl[1].cool;	// Set temp (deg C)
		secsref = secs;			// Measure time of next phase
		state = 14;
	
	case 14: // Cool down from Ferment phase
		fret = loop(therm_ctl, therm_shell,  0); 	// Update control loop
		if ( fret < 4.0 ) break;
		// Unlikely we ever get here without cold temp source
		loopctl_setfan(0);	// Fan OFF
		state = 0;		// Idle until hapless Op signals 
		break;
	default:
		printf("#### EGADS! Bad case number: %d\n\r", state);
		state = -1;
	}
	/* Return a pointer to some stuff to entertain the hapless Op. */
	loopvals.duration = duration;
	loopvals.state = state;
	loopvals.setpoint = setpoint;
	loopvals.secs = secs;
	loopvals.secs0 = secs0;
	loopvals.ipwm = ipwm;
	loopvals.therm_ctl = therm_ctl;
	loopvals.secsref = secsref;
	loopvals.looperr = (float)looperr;
	loopvals.derivative = (float)derivative;
	loopvals.integral = (float)integrator_tmp;
	loopvals.derivative_a = (float)derivative_a;

	return &loopvals;
	
}
