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

/*
Key to locating file with source code--
@1  = svn_pod/sw_stm32/trunk/lib/libmiscstm32/busfreq.c
@2  = svn_pod/sw_stm32/trunk/devices/32KHz_p1.c
@3  = svn_pod/sw_pod/trunk/pod_v1/p1_initialization.c
@4  = svn_pod/sw_stm32/trunk/devices/Tim2_pod.c
@5  = svn_pod/sw_stm32/trunk/lib/libsupportstm32/tickadjust.c
@6  = svn_pod/sw_stm32/trunk/lib/libsupportstm32/ad7799_filter.c 
@7  = svn_pod/sw_stm32/trunk/devices/rtc_timers.c 
@8  = svn_pod/sw_stm32/trunk/lib/libsupportstm32/ad7799_packetize.c 
@9  = svn_pod/sw_pod/trunk/pod_v1/common.c 
@10 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_convert.c
@11 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_packetize.c 
@12 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/pushbutton_packetize.c 
@13 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/adc_packetize.c 
@14 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/adc_filter.c 
@15 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/calibration.c 
@16 = svn_pod/sw_pod/trunk/pod_v1/p1_shutdown.c
@17 = svn_pod/sw_pod/trunk/pod_v1/p1_normal_run.c
@18 = svn_pod/sw_pod/trunk/pod_v1/p1_PC_monitor_readout.c

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
#include "libmiscstm32/systick1.h"		// SYSTICK routine
#include "libmiscstm32/printf.h"		// Tiny printf

/* The following are in 'svn_pod/sw_stm32/trunk/lib/libusartstm32' */
#include "libusartstm32/usartallproto.h"	// For USART1, UART4


/* These hold SYSTICK 32b times.  Generally used for assuring initialization delays
have been met.  main ('pod_v1.c') allocates the struct.  Times are set during the 
initialization and later tested as appropriate.  It is up to the routines not to step
on someone elses time. */
struct TICKTIMERS
{
	unsigned int	t0;		
	unsigned int	t1;	
	unsigned int	t2;	
	unsigned int	t3;	
	unsigned int	t4;	
};

/* This struct is used to return two int's from a subroutine */
struct TWO
{
	int	n1;
	int	n2;
};

/* This struct is used to return a pointer and count from a "get packet" routine */
struct PKT_PTR
{
	char*		ptr;	// Pointer to packet
	unsigned int	ct;	// Byte count of packet
};

/* The following is used for loading the RTC registers */
struct RTCREG1
{
	unsigned int prl;	// Prescaler divide count (-1 adjustment made in load routine)
	unsigned int cnt;	// Counter register
	unsigned int alr;	// Alarm register
};

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


/* RTC time counts: union ties together different lengths */
union LL_L_S	
{
	unsigned long long	  ull;		// 64 bit version unsigned
	signed	 long long         ll;		// 64 bit version signed
	unsigned int		ui[2];		// Two 32 bit'ers unsigned
	int			 n[2];		// Two 32 bit'ers signed
	unsigned long		ul[2];		// Two 32 bit'ers unsigned
	signed   long           sl[2];		// Two 32 bit'ers signed
	unsigned short		us[4];		// Four 16 bit unsigned
	signed   short		 s[4];		// Four 16 bit signed
	unsigned char		uc[8];		// Last but not least, 8b unsigned
	signed   char		 c[8];		// And just in case, 8b signed
};
/* Different ways of dealing with an int */
union UI_I_C
{
	unsigned int		ui;
	signed int		n;
	unsigned short		us[2];
	signed short		s[2];
	unsigned char		uc[4];
	signed char		c[4];
};
/* Use for three byte (signed) tension readings */
union C3
{
	signed char	c[3];
	unsigned char	uc[3];
};

/* Time counts used by all */
struct ALLTIMECOUNTS
{
	union LL_L_S		SYS;		// Tick-count that extends RTC_CNT precision, but includes freq error slippage
	union LL_L_S		DIF;		// Difference between RTC_CNT and ((SYS.ul[0] << 2)|(sPreTick & 0x03))
	union LL_L_S		LNX;		// Linux time count (tick-count + llDiff) <== used for packet time stamping
	union LL_L_S		GPS;		// GPS: linux time divined from the ascii of last GPS fix
	union LL_L_S		TIC;		// GPS: SYS tick-count stored, by 1 PPS input capture (TIM2)
	union LL_L_S		UPD;		// Temporary update	

	unsigned int	uiNextTickAdjTime;	// Next tick-time to adjust for error (@1)
	unsigned int	uiTim2diff;		// Number of processor ticks between 1_PPS (TIM2) interrupts
	unsigned int	uiTim1diff;		// Number of processor ticks between 8192 RTC osc interrupts
		
	int		nTickAdjust;		// Number of ticks to get back on time
	int		nTickAdjustRTC;		// Number of ticks to get back on time under RTC interrupt
	int 		nTickErrAccum;		// Running accumulation of error (@1)
	int		nOscOffset32;		// "Fixed" offset of oscillator 32 KHz
	int		nOscOffset8;		// "Fixed" offset of oscillator 8 MHz
	int		nOscOffFilter;		// cic filtered offset
	int		nPolyOffset32;		// Polynomial computed osc error 32 KHz osc
	int		nPolyOffset8;		// Polynomial computed: 8 Mhz osc-- (ticks per sec - 48E6)
	int		uiAdcTherm;		// ADC thermister
	int		uiThermtmp;		// Temperature (deg C*100)
	short		sPreTick;		// Divides TR_CLK (32768/4) to give polling ticks (32768/16)
	short		sPreTickTIC;		// GPS: sPreTick saved upon 1 PPS interrupt

};


#define GPSARRAYSIZE	12	// Size of SSMMHHDDMMYY

struct TIMESTAMPGP1
{
	unsigned char	id;		// ID of packet
	char g[GPSARRAYSIZE];		// GPS time in ASCII array: SSMMHHDDMMYY(sec, min, hour, day, month, year)
	struct ALLTIMECOUNTS alltime;	// All the time counters and differences
	int	filler;
};

#define	PKT_AD7799_TENSION_SIZE		16	// Number of tension readings in a packet 
struct PKT_AD7799
{
	unsigned char	id;				// Packet ID
	union LL_L_S 	U;				// 64 bit Linux time
	int 	tension[PKT_AD7799_TENSION_SIZE];	// Filtered AD7799 readings
};


/* Packet for selected CIC filtered ADC readings for accelerometer X,Y,Z */
#define NUMBERACCELREADINGSPKT	3		// Number of readings for one group in this packet
#define	PKT_ACCEL_GRP_SIZE	16		// Number of groups of ADC readings in a packet 

struct PKT_ACCEL
{
	unsigned char	id;				// Packet ID
	union LL_L_S 	U;				// 64 bit Linux time
	unsigned short 	adc[NUMBERACCELREADINGSPKT][PKT_ACCEL_GRP_SIZE];	// CIC filtered readings
};

/* Packet for selected averaged ADC readings for battery & thermistor  */
#define NUMBERADCREADINGSPKT	3		// Number of ADC readings in this packet

struct PKT_BATTMP
{
	unsigned char	id;				// Packet ID
	union LL_L_S 	U;				// 64 bit Linux time
	unsigned short 	adc[NUMBERADCREADINGSPKT];	// Averaged adc readings
};

/* Packet for pushbutton */
struct PKT_PUSHBUTTON
{
	unsigned char	id;		// Packet ID
	union LL_L_S 	U;		// 64 bit Linux time
};


/* This is used by 'p1_PC_monitor_readout.c' which gets packets from the SD Card, prepares & sends them to the USART */
#define PACKETREADBUFSIZE	128	// Must be bigger than the largest packet

union PKT_ALL
{
	char packet_read_buf[PACKETREADBUFSIZE];// Hold packet read from SD card
	struct PKT_AD7799 pkt_ad7799;		// ID 1 Tension 
	struct TIMESTAMPGP1 timestampg;		// ID 2 GPS v rtc tickcounter
	struct PKT_BATTMP batttmp;		// ID 3 Battery & temperature
	struct PKT_PUSHBUTTON pushbutton;	// ID 4 Pushbutton
	struct PKT_ACCEL  accel;		// ID 5 Accelerometer
};

/* Calibration struct is stored in SD card extra blocks area */
#include "calibration.h"
#include "sdcard.h"		// sdcard read/write
union SDCALBLOCK0
{
	char sdcalblock[SDC_DATA_SIZE];	// One SD card block
	struct CALBLOCK calblock;	// Calibration values
	unsigned int crc;		// CRC on calblock

};


/* The following are in 'svn_pod/sw_stm32/trunk/devices' */
#include "32KHz_p1.h"		// RTC & BKP routines
#include "ad7799_comm.h"	// ad7799 routines (which use spi1 routines)
#include "adcpod.h"		// ADC1 routines
//#include "gps_1pps.h"		// Needed for Tim2 routine
#include "PODpinconfig.h"	// gpio configuration for board
#include "pwrctl.h"		// Low power routines
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



/* The following are in 'svn_pod/sw_pod/trunk/pod_v1' */
#include "LED_ctl.h"		// Pulsing external LED (on BOX)
#include "p1_initialization.h"	// Some things for the following
#include "pod_v1.h"		// Tiny main routine
#include "RS232_ctl.h"		// MAX232, and USART1, UART4 bring-up & initialization
#include "p1_shutdown.h"	// Things having to do for shutting down to deepsleep
#include "p1_logging_handler.h"	// Things having to do with logging
#include "p1_PC_handler.h"	// Things having to do with PC comm
#include "p1_PC_monitor.h"	// More PC comm stuff
#include "p1_PC_monitor_accel.h"// Output current batt|temp to PC
#include "p1_PC_monitor_batt.h"	// Output current batt|temp to PC
#include "p1_PC_monitor_gps.h"	// Output current gps to PC
#include "p1_PC_monitor_readout.h"// Output readback of packets from SD Card
#include "p1_gps_time_linux.h"	// Convert gps to linux time
#include "p1_gps_time_convert.h"	// Extraction of time from GPS records
#include "ad7799_packetize.h"	// ad7799 readings packetizing (tension)
#include "adc_packetize.h"	// ADC readings packetizing (accelerometer; battery|thermistor)
#include "gps_packetize.h"	// GPS packetizing
#include "pushbutton_packetize.h"// Pushbutton stuff
#include "p1_normal_run.h"	//
#include "p1_calibration_sd.h"	// Retrieval/storing of calibrations on SD card
#include "p1_PC_monitor_inputcal.h"// Input new calibration value 




/* Shutdown timing */
/* Temp short time for debugging */
//#define INACTIVELOGGINGTICKCOUNT 60*ALR_INCREMENT // Shutdown timeout counter reset count (seconds) (@2)) (@16) (@17)

/* This is a more reasonable time */
#define INACTIVELOGGINGTICKCOUNT 10*3600*ALR_INCREMENT // Shutdown timeout counter reset count (seconds) (@2)) (@16) (@17)

/* This sets the deep sleep time */
// For 'shutdownflag' = 1   Normal deepsleep with periodic wake-up to adjust time for temperature
#define DEEPSLEEPTICKCOUNT1	    10*RTC_TICK_FREQ // Count for duration of deepdleep STANBY (seconds * ticks/seconds (@2)) (@16)
// For 'shutdownflag' = 2   Battery low deepsleep--very long time, and depends on pushbutton to wakeup
#define DEEPSLEEPTICKCOUNT2	    -1	 // Count for duration of deepdleep STANBY (seconds * ticks/seconds (@2)) (@16)


/* Get pushbutton status quickly in case it goes away (@3) */
extern char cPA0_reset;	// Status of Pushbutton (PA0) coming out of reset


/* PACKET ID assignments */
#define NUMBERPKTTYPES	5	// Number of packet types
#define ID_AD7799	1	// (@8)
#define ID_GPS		2	// (@11)
#define	ID_BATTTEMP	3	// (@13)
#define	ID_PUSHBUTTON	4	// (@12)
#define ID_ACCEL	5	// (@13)





/* 0 = normal; 1 = tidy up, it is time to shutdown */
extern short shutdownflag;


/* SYSTICK timers used at various places (@6) */
extern struct TICKTIMERS sys;	// Systick times (other routines will reference these) (@1)

extern unsigned char ad7799_8bit_reg;		// Byte with latest status register from ad7799_1
extern unsigned char ad7799_8bit_wrt;		// Byte to be written
extern volatile unsigned char ad7799_comm_busy;// 0 = not busy with a sequence of steps, not zero = busy
extern unsigned short ad7799_16bit_wrt;	// 16 bits to be written

/* These 'unions' are used to read bytes into the correct order for mult-byte registers.
The AD7799 sends the data MSB first, so each byte needs to be stored in reverse order */
extern union SHORTCHAR	ad7799_16bit;
extern union INTCHAR	ad7799_24bit;



#endif


