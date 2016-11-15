/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_common.h
* Author             : deh
* Date First Issued  : 09/02/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Common variables
*******************************************************************************/
/*
09-02-2011
*/

#ifndef __P1_COMMON
#define __P1_COMMON

#include "p1_gps_1pps.h"	// Needed for Tim2 routine


/* The following are for newlib/system */
#include <time.h>
#include <stdlib.h>

/* The following are in 'svn_pod/sw_stm32/trunk/lib/libopenstm32' */
#include "libopenstm32/gpio.h"	// For gpio pins
#include "libopenstm32/bkp.h"	// #defines for addresses of backup registers
#include "libopenstm32/pwr.h"	// Power control registers

/* The following are in 'svn_pod/sw_stm32/trunk/lib/libmiscstm32' */
#include "libmiscstm32/clockspecifysetup.h"	// For clock setup
//#include "libmiscstm32/systick1.h"		// SYSTICK routine
#include "libmiscstm32/printf.h"		// Tiny printf

/* The following are in 'svn_pod/sw_stm32/trunk/lib/libusartstm32' */
#include "libusartstm32/usartallproto.h"	// For USART1, UART4

#include "common_misc.h"

/* This struct is used to return two int's from a subroutine */
struct TWO
{
	int	n1;
	int	n2;
};

/* This struct is used to return a pointer and count from a "get packet" routine */
//struct PKT_PTR
//{
//	char*		ptr;	// Pointer to packet
//	unsigned int	ct;	// Byte count of packet
//};


union TIMCAPTURE64
{
	unsigned short	us[4];
	unsigned int	ui[2];
	unsigned long long ll;
};

struct TIMCAPTRET32
{
	unsigned int 	ic;
	unsigned int	flg;
	unsigned int	cnt;	// RTC_CNT (in memory mirror)
};



/* The following are in 'svn_pod/sw_stm32/trunk/devices' */
#include "32KHz_p1.h"		// RTC & BKP routines
#include "ad7799_comm.h"	// ad7799 routines (which use spi1 routines)
#include "adcpod.h"		// ADC1 routines
//#include "gps_1pps.h"		// Needed for Tim2 routine
#include "PODpinconfig.h"	// gpio configuration for board
#include "spi1ad7799.h"		// spi1 routines 
#include "bit_banding.h"	// Macro for using libopenstm32 #defines for bit banding
#include "hwcrc.h"		// Hardware CRC-32 



/* The following are in 'svn_pod/sw_stm32/trunk/lib/libsupportstm32' */
#include "ad7799_filter.h"	// ad7799 filtering
#include "adctherm_cal.h"	// Conversion of thermistor adc reading to temp (deg C * 100)
#include "calibration.h"	// calibrations for various devices on pod board
#include "cic_filter_L64_D32.h"	// CIC filtering of ADC readings
#include "cic_filter_ll_N2_M3.h"// CIC filtering of tension for monitoring
#include "cic_filter_l_N2_M3.h"// CIC filtering of tension for monitoring
#include "rtctimers.h"		// RTC countdown timers
#include "sdlog.h"		// Logging routines
#include "tickadjust.h"		// Time adjust and init/shutdown aspects



/* The following are in current directory */
#include "p1_gps_time_linux.h"	// Convert gps to linux time
#include "p1_gps_time_convert.h"	// Extraction of time from GPS records


/* PACKET ID assignments */
// There must be enough entries in the table in 'p1_logging_handler.c' for the NUMBERPKTTYPES */
#define NUMBERPKTTYPES	6	// Number of packet types
#define ID_AD7799	1	// (@8)
#define ID_GPS		2	// (@11)
#define	ID_BATTTEMP	3	// (@13)
#define	ID_PUSHBUTTON	4	// (@12)
#define ID_ACCEL	5	// (@13)
#define ID_GPSFIX	6	// (@11)


/* SYSTICK timers used at various places (@6) */
extern struct TICKTIMERS sys;	// Systick times (other routines will reference these) (@1)



#endif


