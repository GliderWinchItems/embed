/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : ad7799_packetize.h
* Hacker	     : deh
* Date First Issued  : 09/04/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Buffering/unbuffering AD7799 filtered readings
*******************************************************************************/
#ifndef __AD7799_PACKETIZE
#define __AD7799_PACKETIZE


#define	PKT_AD7799_TENSION_SIZE		16	// Number of tension readings in a packet 
#define PKT_AD7799_BUF_CT		4	// Number of PKT_AD7799 structs buffered
#define PKT_AD7799_ID		ID_AD7799	// Packet ID
#define AD7799FILTERGAIN	18		// Number of bits to right-shift filter output

struct PKT_AD7799
{
	unsigned char	id;					// Packet ID
	union LL_L_S 	U;					// 64 bit Linux time
	unsigned int 	tension[PKT_AD7799_TENSION_SIZE];	// Filtered AD7799 readings
};

/******************************************************************************/
void ad7799_packetize_init(void);
/* @brief	: Set id into array (so we don't have to do it each time later)
 ******************************************************************************/
void ad7799_packetize_add(void);
/* @brief	: Add a AD7799 reading to the packet buffer (enter from RTC interrupt chain)
 ******************************************************************************/
struct PKT_PTR ad7799_packetize_get_main(void);
/* @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR ad7799_packetize_get_monitor(void);
/* @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR ad7799_packetize_get_shutdown(void);
/* @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/

/* Address ad7799_packetize_add will go to upon completion (@8) */
extern void 	(*rtc_ad7799_packetizedone_ptr)(void);// Address of function to call to go to for further handling under RTC interrupt



#endif

