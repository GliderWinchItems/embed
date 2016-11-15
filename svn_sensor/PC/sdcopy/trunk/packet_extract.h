/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : packet_extract.h
* Hackeroos          : deh
* Date First Issued  : 06/14/2013
* Board              : any
* Description        : Retrieve packet from a sd block
*******************************************************************************/

#ifndef __PACKET_EXTRACT
#define __PACKET_EXTRACT

#include "common_can.h"

#define	SDCARD_BLOCKSIZE		512
#define	SDLOG_DATASIZE			(512-8)

struct PKTP
{
	int	ct;		// Packet size
	unsigned char*	p;	// Pointer to packet
	unsigned long long pid;
};
struct	LOGBLK			/* Used for local structured access */
{
	char data[SDLOG_DATASIZE];	// One block minus space for long long at end
	unsigned long long pid;		// Last 8 bytes holds the PID
	int blk;			// Nice place for the block number
};
union BLOCK			/* Union of raw & structured blocks */
{
	char raw[SDCARD_BLOCKSIZE];			/* Buffer for "sdcard.[ch]" calls */
	struct LOGBLK str;
};



/*******************************************************************************/
int packet_extract_first(struct PKTP *pktp, unsigned char* pblk);
/* @brief 	: Extract PID, find last packet in block
 * @param	: pointer to struct with packet count, pointer, and block pid
 * @param	: pblk--pointer to control block
 * @return	: 0 = OK
*******************************************************************************/
int packet_extract_next(struct PKTP *pktp);
/* @param	: pointer to struct with packet count, pointer, and block pid
 * @return	: 0 = OK
*******************************************************************************/
int sdblk_allzero(unsigned char * p);
/* @param	: pointer to sd block
 * @return	: 0 = all zero; plus = count of non-zero bytes
*******************************************************************************/
void packet_convert(struct CANRCVTIMBUF* pout, struct PKTP *pp);
/* @brief 	: convert packet to CAN type struct
 * @param	: pblk--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/
int getatime(int blk, struct PKTP *pktp);
/* @brief	: Read block and extract any time msg if present
 * @param	: blk: block number 
 * @return	:  1 = msg time < search time;
 *		:  0 = msg time >= search time;
 *		: -1 = large number examined and no time msg found
 ******************************************************************************/


#endif

