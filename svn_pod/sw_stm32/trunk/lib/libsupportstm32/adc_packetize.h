/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : adc_packetize.h
* Hacker	     : deh
* Date First Issued  : 09/06/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Buffering/unbuffering adc averaged readings
*******************************************************************************/
#ifndef __ADC_PACKETIZE
#define __ADC_PACKETIZE

#include "../../../../sw_pod/trunk/pod_v1/p1_common.h"

/* 
If the ADC readings are filtered, they will limited (via scaling) to 16 bits max, i.e. 2 bytes
per readings 
*/
/* ----------------- Accelerometer -----------------------------------------------------*/
#define NUMBERACCELREADINGSPKT	3		// Number of readings for one group in this packet
#define	PKT_ACCEL_GRP_SIZE	16		// Number of groups of ADC readings in a packet 
#define PKT_ACCEL_BUF_CT	4		// Number of PKT_ACCEL structs buffered
#define ADCFILTERGAIN		18		// Number of bits to right-shift filter output
#define	ACCELDISCARDNUMBER	4		// Count of initial readings to discard
#define ACCELDECIMATE_A		16		// Decimation count for first CIC stage
#define ACCELDECIMATE_B		8		// Decimation count for second CIC stage
#define ACCELADCGAIN_A		15		// Number of bit shifts to scale down gain for 1st stage
#define ACCELADCGAIN_B		12		// Number of bit shifts to scale down gain for 2nd stage


/* Packet for selected CIC filtered ADC readings for accelerometer X,Y,Z */
struct PKT_ACCEL
{
	unsigned char	id;				// Packet ID
	union LL_L_S 	U;				// 64 bit Linux time
	unsigned short 	adc[NUMBERACCELREADINGSPKT][PKT_ACCEL_GRP_SIZE];	// CIC filtered readings
};

/* ----------------- Battery & Thermistor ---------------------------------------------- */
#define PKT_ACCEL_ID		ID_BATTTEMP	// Packet ID
#define ADCAVERAGEORDER		16		// Power of two for number of readings to average
#define ADCNUMBERINAVERAGE	(1<<ADCAVERAGEORDER)	// Number in average for SD card
#define ADCORDERINAVERAGEMONITOR	9	// Power of two for number in average for monitoring on PC
#define ADCAVERAGESCALING	2		// Bits of resolution to add due to SD averaging
#define NUMBERADCREADINGSPKT	3		// Number of ADC readings in this packet

/* adc[NUMBERADCREADINGSPKT]--
[0] PA 3 ADC123-IN3	Thermistor on 32 KHz xtal
[1] PB 0 ADC12 -IN8	Bottom cell of battery
[2] PB 1 ADC12 -IN9	Top cell of battery 
*/

/* Packet for selected averaged ADC readings for battery & thermistor  */
struct PKT_BATTMP
{
	unsigned char	id;				// Packet ID
	union LL_L_S 	U;				// 64 bit Linux time
	unsigned short 	adc[NUMBERADCREADINGSPKT];	// Averaged adc readings
};

/******************************************************************************/
void adc_packetize_init(void);
/* @brief	: Set id into array (so we don't have to do it each time later)
 ******************************************************************************/
void adc_packetize(void);
/* @brief	: Add a ADC reading to the packet buffer
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_accel(void);
/* @brief	: Get pointer & count to the buffer to be drained--accelerometer readings
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
int * adc_packetize_get_accel_monitor(void);
 /* @brief	: Get pointer & count to the buffer to be drained--accelerometer readings
  * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_battmp(void);
/* @brief	: Get pointer & count to the buffer to be drained--battery & thermistor readings
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR adc_packetize_get_battmp_monitor(void);
/* @brief	: Get pointer & count to the buffer to be drained--battery & thermistor readings
 ******************************************************************************/

/* Address adc_filter.c routine will go to upon completion (@14) */
extern void 	(*adc_filterdone_ptr)(void);		// Address of function to call to go to for further handling under RTC interrupt

/* Packet that will written to the SD Card (@13) */
extern struct PKT_ACCEL accel_pktbuf[PKT_ACCEL_BUF_CT];

/* Packet that will written to the SD Card (@13) */
//extern struct PKT_BATTMP batttemp_pktbuf;	//

#endif

