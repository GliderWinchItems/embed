/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : packet_extract.h
* Hackeroos          : deh
* Date First Issued  : 06/14/2013
* Board              : any
* Description        : Retrieve packet from a sd block
*******************************************************************************/

#ifndef __PACKET_EXTRACT
#define __PACKET_EXTRACT

#define	SDCARD_BLOCKSIZE		512
#define	SDLOG_DATASIZE			(512-8)

struct PKTP
{
	int	ct;	// Packet size
	unsigned char*	p;	// Pointer to packet
	unsigned long long pid;
};
struct	LOGBLK						/* Used for local structured access */
{
	char data[SDLOG_DATASIZE];
	unsigned long long pid;
};
union BLOCK							/* Union of raw & structured blocks */
{
	char raw[SDCARD_BLOCKSIZE];			/* Buffer for "sdcard.[ch]" calls */
	struct LOGBLK str;
};

union CANDATA	// Unionize for easier cooperation amongst types
{
	unsigned long long ull;
	signed long long   sll;
	unsigned int 	   ui[2];
	unsigned char	   uc[8];
};
struct CANRCVBUF		// Combine CAN msg ID and data fields
{
	unsigned int id;	// 0x00 CAN_TIxR: mailbox receive register ID p 662
	unsigned int dlc;	// 0x04 CAN_TDTxR: time & length p 660
	union CANDATA cd;	// 0x08,0x0C CAN_TDLxR,CAN_TDLxR: Data payload (low, high)
};
struct CANRCVTIMBUF		// CAN data plus timer ticks
{
	unsigned long long	tim;	// Linux time, offset, in 1/64th ticks
	struct CANRCVBUF 	R;	// CAN data
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


#endif

