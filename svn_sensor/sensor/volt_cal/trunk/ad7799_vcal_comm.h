/******************************************************************************
* File Name          : ad7799_vcal_comm.h
* Date First Issued  : 10/04/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for operating AD7799_1
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AD7799_COMM_VCAL
#define __AD7799_COMM_VCAL

#include <stdint.h>

/* ------------- Configuration register --------------------------------------*/
/* Channel selection  ("$" = reset setting) */
#define AD7799_CH_1	0x0	//$AIN1(+) - AIN1(-)  Calibration pair 0
#define AD7799_CH_2	0x1	// AIN1(+) - AIN1(-)  Calibration pair 1
#define AD7799_CH_3	0x2	// AIN1(+) - AIN1(-)  Calibration pair 2
#define AD7799_CH_1X	0x3	// AIN1(-) - AIN1(-)  Calibration pair 0
#define AD7799_CH_AV	0x7	// Auto select gain = 1/6, and internal 1.17v ref
/* Buffer selection */
#define AD7799_BUF_IN	(1<<4)	// Select buffer amp (only for gains 1x, 2x)
/* Reference detection function */
#define AD7799_REF_DET	(1<<5)	// Reference detection ON
/* Gain selection */
#define AD7799_1NO	0	// gain 1   max input = 2.5v  (No in-amp used)
#define AD7799_2NO	(1<<8)	// gain 2   max input = 1.25v (No in-amp used)
#define AD7799_4	(2<<8)	// gain 4   max input = 625mv
#define AD7799_8	(3<<8)	// gain 8   max input = 312.5mv
#define AD7799_16	(4<<8)	// gain 16  max input = 156.2mv
#define AD7799_32	(5<<8)	// gain 32  max input = 78.125mv
#define AD7799_64	(6<<8)	// gain 64  max input = 39.06mv
#define AD7799_128	(7<<8)	// gain 128 max input = 19.53mv
/* Unipolar/bipolar */
#define AD7799_UNI	(1<<12)	// zero differential = 0x000000; full scale == 0xffffff
/* Burn out current detection */
#define AD7799_BOUT	(1<<13)	// 100nA current sources in signal path enabled

/* --------------- Mode register ---------------------------------------------*/
#define AD7799_CONT		(0<<13)	// Continuous conversion
#define AD7799_SINGLE		(1<<13)	// Single conversion then power down
#define AD7799_IDLE		(2<<13)	// Idle: Modulator clocks run, modulator and ADC filter held reset
#define AD7799_PWRDN		(3<<13)	// Power down: circuitry powered down including burnout currents
#define AD7799_ZEROCALIB	(4<<13)	// Internal calibrate: short input, set offset register, then place in idle
#define AD7799_FSCALIB		(5<<13)	// Full scale calibrate: connect to input, calib, place in idle (not applicable to 128 gain)
#define AD7799_SYSZEROCALIB	(6<<13)	// System zero-scale calibrate: measure with input zero (no load on load cell) & set register
#define AD7799_SYSFSCALIB	(7<<13)	// System full-scale calibrate: measure will full scale load & set register
/* Output FET */
#define AD7799_PSW		(1<<12)	// Open-drain FET ON.
/* Conversion update rate ( 'p' in the following is for a decimal point) */
//             fADC (Hz)       tSETTLE (ms)   Rejection @ 50 Hz/60 Hz */
#define AD7799_Reserved	0	// ...
#define AD7799_470SPS	1 	//   4   2.12 ms per 
#define AD7799_242SPS	2	//   8   4.13
#define AD7799_123SPS	3	//  16   8.13
#define AD7799_62SPS	4	//  32  16.1
#define AD7799_50SPS	5	//  40  20.0
#define AD7799_39SPS	6	//  48  25.6
#define AD7799_33p2SPS	7	//  60  30.1
#define AD7799_19p6SPS	8	// 101  52.0    90 dB (60 Hz only)
#define AD7799_16p7SPS	9	// 120  59.9    80 dB (50 Hz only)
#define AD7799_16p7bSPS	10	// 120  59.9    65 dB
#define AD7799_12p5SPS	11	// 160  80.0    66 dB
#define AD7799_10SPS	12	// 200 100      69 dB
#define AD7799_8p33SPS	13	// 240 120      70 dB
#define AD7799_6p25SPS	14	// 320 160      72 dB
#define AD7799_4p17SPS	15	// 480 240      74 dB

/* ----------------- Status register -----------------------------------------*/
#define AD7799_NRDY	(1<<7)	// Not Ready: goes low when data written to data register
#define AD7799_ERR	(1<<6)	// Data reg has been clamped to zero. Overrange, underange.  Cleared by start conversion
#define AD7799_NREF	(1<<5)	// No Reference Bit.  Reference voltage is below limit.
#define AD7799_CHSET	(0x07)	// Channel setting (see configuration register)




/******************************************************************************/
void ad7799_vcal_rd_8bit_reg (unsigned char);
/* @brief	: Read an 8 bit register from ad7799
 * @param	: register code
*******************************************************************************/
void ad7799_vcal_rd_IO_reg (void);
/* @brief	: Read IO register from ad7799
******************************************************************************/
void ad7799_vcal_rd_ID_reg (void);
/* @brief	: Read ID register from ad7799
******************************************************************************/
void ad7799_vcal_reset (void);
/* @brief	: Send 32 consecutive 1's to reset the digital interface
 * @return	: None now 
*******************************************************************************/
void ad7799_vcal_rd_16bit_reg (unsigned char);
/* @brief	: Read an 16 bit register from ad7799
 * @param	: register code
******************************************************************************/
void ad7799_vcal_rd_24bit_reg (unsigned char);
/* @brief	: Read an 24 bit register from ad7799
 * @param	: register code
*******************************************************************************/
void ad7799_vcal_wr_16bit_reg (unsigned char);
/* @brief	: Write an 16 bit register from ad7799
 * @param	: register code
*******************************************************************************/
void ad7799_vcal_set_continous_rd (void);
/* @brief	: Writes 0x5c to the comm register to set part in continuous read mode
*******************************************************************************/
void ad7799_vcal_exit_continous_rd (void);
/* @brief	: Writes 0x58 to the comm register to exit continuous read mode
*******************************************************************************/
void ad7799_vcal_rd_configuration_reg (void);
/* @brief	: Read 16 bit configuration register from ad7799
*******************************************************************************/
void ad7799_vcal_wr_configuration_reg (unsigned short usX);
/* @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit configuration register to ad7799
*******************************************************************************/
void ad7799_vcal_rd_mode_reg (void);
/* @brief	: Read 16 bit mode register from ad7799
*******************************************************************************/
void ad7799_vcal_wr_mode_reg (unsigned short usX);
/* @param	: usX = register value (note: routine takes care of byte order)
 * @brief	: Write 16 bit mode register to ad7799
*******************************************************************************/
char ad7799_vcal_1_RDY(void);
/* @brief	: Initialize AD7799_1 registers for strain gauge; /CS left in low state
 * @param	: ucSPS: Sampling rate code (see ad7799_vcal_comm.h file)
*******************************************************************************/
void ad7799_vcal_rd_offset_reg (void);
/* @brief	: Read 24 bit mode register from ad7799
*******************************************************************************/
void ad7799_vcal_rd_fullscale_reg (void);
/* @brief	: Read 24 bit mode register from ad7799
*******************************************************************************/
void ad7799_vcal_signextend_24bit_reg (void);
/* @brief	: Convert 3 byte readings to 4 byte signed
*******************************************************************************/
void ad7799_vcal_rd_data_reg (void);
/* @brief	: Read 24 bit mode register from ad7799
*******************************************************************************/
void ad7799_vcal_rd_status_reg (void);
/* @brief	: Read status register from ad7799
*******************************************************************************/
int32_t ad7799_vcal_rd_24bit_polar(uint16_t us);
/* @brief	: Read an 24 bit register from ad7799
 * @param	: us = unipolar/bipolar: 0 = bipolar, not zero = unipolar
 * @return	: int32_t with fixed up reading
*******************************************************************************/
uint16_t ad7799_vcal_rd_16bit_swab(void);
/* @brief	: Put received bytes in little endian order
 * @return	: 16b register in usable byte order
*******************************************************************************/

/* Macros that help getting the selection codes right (for bozos like the author) */
#define AD7799_RD_STATUS_REG		(0 << 3)	// Register selection code: 000
#define AD7799_RD_MODE_REG		(1 << 3)	// Register selection code: 001
#define AD7799_RD_CONFIGURATION_REG	(2 << 3)	// Register selection code: 010
#define AD7799_DATA_REG			(3 << 3)	// Register selection code: 011
#define AD7799_ID_REG			(4 << 3)	// Register selection code: 100
#define AD7799_IO_REG		  	(5 << 3)	// Register selection code: 101
#define AD7799_RD_OFFSET_REG		(6 << 3)	// Register selection code: 110
#define AD7799_RD_FULLSCALE_REG		(7 << 3)	// Register selection code: 111

#define AD7799_SIGNEXTEND_24BIT_REG	(ad7799_vcal_24bit.s[1] = ad7799_vcal_24bit.c[2])	// Sign extend the high order byte

/* These 'unions' are used to re-order the bytes into the correct order for mult-byte register reading.
The AD7799 sends the data MSB first, so each byte needs to be stored in reverse order for this machine. */
union SHORTCHAR
{
	unsigned short us;
	unsigned char uc[2];
};
union INTCHAR
{
	signed int	n;	// 32 bit signed int for final result
	unsigned char	uc[4];	// Three bytes from AD7799 are stored here
	signed char	c[4];	// Signed char
	signed short	s[2];	// Signed short for extending hi-order byte after re-ordering
	
};


extern volatile unsigned char ad7799_vcal_comm_busy; // 0 = not busy with a sequence of steps, not zero = busy
extern void ad7799_vcal_rd_24bit_reg2(void);


#endif 
