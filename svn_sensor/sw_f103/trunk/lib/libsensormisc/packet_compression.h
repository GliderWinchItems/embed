/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : packet_compression.h
* Hackeroos          : deh
* Date First Issued  : 12/16/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for compressing sensor packets
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSOR_PACKET_COMPRESSION
#define __SENSOR_PACKET_COMPRESSION

#include "common.h"
#include "common_lsensormisc.h"
#include "common_misc.h"
#include "../../../../../svn_common/trunk/common_can.h"

/*
See the following for detailed layout--
../svn_sensor/docs/trunk/proposal/packet_compression_scheme.txt
*/


/* Set of circular buffers */
#define NUMPKTBUFS	4		

/* Repetition count bit field length */
#define PACKPKTREPCTSZ		4	// Number of bits in repetition count (0-15)

/*
See the following for detailed layout--
../svn_sensor/docs/trunk/proposal/packet_compression_scheme.txt
*/

/* Size is designated in words */
#define PKT_PACKEDSIZE		(250/4) 	// Fit a 250 byte max SD card packet size
#define PACKPKTREPCTSZ		4	// Number of bits in repetition count (0-15)

/*
See the following for detailed layout--
../svn_sensor/docs/trunk/proposal/packet_compression_scheme.txt
*/
#define ID	0
#define TIME	2
#define	ENTRYCT	7
union PKT_PACKET
{
	unsigned long		ul[PKT_PACKEDSIZE];
	unsigned short		us[PKT_PACKEDSIZE*2];
	unsigned char		uc[PKT_PACKEDSIZE*4];
};



/* Packet Control Block holds points and stuff for packing/unpacking packets */
/* NOTE: If control block must have initial values for many of the values! */
struct PKT_CTL_BLOCK
{
	void * 		forward;	// Linked list: address of next control block (with a larger id_can)
	int		id_pkt;		// Packet type ID
	union LL_L_S 	U;		// Linux time of lastest reading in 1/64th secs
	int		idx_bufi;	// Index into packet buffer (addition, maybe under interrupt)
	int		idx_bufm;	// Index into packet buffer (removal, likely under mainline polling loop)
	int		reading;	// Reading passed in
	int		cur_val;	// Current value
	int		nDelta;		// Difference between new reading and previous reading
	unsigned int	repct;		// Repetition count (being built)
	unsigned long bitfield;		// Current bit banding address in bitfield
	unsigned long bitfield_end;	// Last bit banding address in current packet
	unsigned int	id_can;		// CAN id in the form we receive it
	/* The following are the packet buffers that get written to the SD card */
	union PKT_PACKET pkt[NUMPKTBUFS];	// A set of packet buffers
	
};


/*******************************************************************************/
void packpkt_init(struct PKT_CTL_BLOCK * pb, struct CANRCVTIMBUF * pcan);
/* @brief 	: Initialize a packet
 * @param	: pb--pointer to control block
 * @param	: pcan--pointer to struct with: time, can ID, can data
 * @return	: Info is in control block	
*******************************************************************************/
void packpkt_add(struct PKT_CTL_BLOCK * pb, struct CANRCVTIMBUF * pcan );
/* @brief 	: Build a packed packet
 * @param	: pb--pointer to control block
 * @param	: pcan--pointer to struct with: time, can ID, can data
 * @return	: Info is in control block	
*******************************************************************************/
struct PKT_PTR packpkt_get(struct PKT_CTL_BLOCK *pb);
/* @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/

#endif 
