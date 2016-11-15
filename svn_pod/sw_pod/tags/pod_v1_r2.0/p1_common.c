/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_common.c
* Author             : deh
* Date First Issued  : 09/02/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Common variables
*******************************************************************************/
#include "adcpod.h"
#include "spi1ad7799.h"
#include "PODpinconfig.h"
#include "rtctimers.h"
#include "libmiscstm32/systick1.h"
#include "libusartstm32/nvicdirect.h" 
#include "libmiscstm32/clockspecifysetup.h"
#include "32KHz_p1.h"

#include "p1_common.h"

/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/devices/adcpod.c
@2 = svn_pod/sw_pod/trunk/pod_v1/p1_initialization.c
@3 = svn_pod/sw_stm32/trunk/devices/32KHz_p1.c
@4 = svn_pod/sw_pod/trunk/pod_v1/p1_shutdown.c
@5 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/pushbutton_packetize.c 
@6 = svn_pod/sw_pod/trunk/pod_v1/common.h
@7 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/pushbutton_packetize.c 
@8 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/calibration_init.c 
@9 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/calibration.h



*/

/* SYSTICK timers used at various places (@6) */
struct TICKTIMERS sys;	// Systick times (other routines will reference these) (@1)
// t1 = resistor divider switch on time
// t2 = adc voltage regulator enable time



/* Get pushbutton status quickly in case it goes away (@2) */
char cPA0_reset;	// Status of Pushbutton (PA0) coming out of reset


/* 0 = normal; 1 = tidy up, it is time to shutdown (@4) */
short shutdownflag;

/* This holds the working values for the calibrations */
struct CALBLOCK strDefaultCalib;	// (@8) (@9)

