/******************************************************************************
* File Name          : common_cmd.c
* Date First Issued  : 11/22/2014
* Board              : PC
* Description        : Routines common to many 'cmd' functions
*******************************************************************************/

#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include <stdio.h>
#include <string.h>
#include <zlib.h>

/* From cangate */
extern FILE *fpList;	// canldr.c File with paths and program names
extern int fdp;		// File descriptor for input file
extern unsigned timeout_ctr;	// Count entries from cangate timeout (1 ms).

static u32 canseqnumber = 0;
/******************************************************************************
 * void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
//static unsigned int RcvMsgCtr = 0;
static struct CANRCVBUF cansave;
static int retryctr = 0;
static int SendMsgCtr = 0;

void sendcanmsg(struct CANRCVBUF* pcan)
{
	struct PCTOGATEWAY pctogateway; 
	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
	cansave = *pcan;	// Save in case of a retry.
	timeout_ctr = 0;	// Reset timeout counter for response
SendMsgCtr += 1;
//printf("MSG # %d: %08x %d %08X %08X\n",SendMsgCtr, pcan->id, pcan->dlc, pcan->cd.ui[0],pcan->cd.ui[1]);
	
	return;
}
/******************************************************************************
 * void sendCMD_simple(u8 cmd, u32 canid);
 * @brief 	: Send a command with only the command code payload byte
 * @param	: Command code (see: common_can.h)
 * @param	: canid = CAN id
*******************************************************************************/
void sendCMD_simple(u8 cmd, u32 canid)
{
	struct CANRCVBUF can;
	can.id = canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.dlc = 1;		// Payload size
	can.cd.ull = 0; // JIC for less debugging confusion
	can.cd.uc[0] = cmd; 	// Add command code
	sendcanmsg(&can);	// Send msg 
	return;	
}
/******************************************************************************
 * void sendCMD_25(u8 cmd, u32 canid, u32 n);
 * @brief 	: Send a command with command code payload byte + a 4 byte number
 * @param	: Command code (see: common_can.h)
 * @param	: canid = CAN id
 * @param	: 4 bytes that goes into payload bytes 2-5, (little endian)
*******************************************************************************/
void sendCMD_25(u8 cmd, u32 canid, u32 n)
{
	struct CANRCVBUF can;
	can.id = canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.dlc = 5;		// Payload size
	can.cd.ull = 0; // JIC for less debugging confusion
	can.cd.uc[0] = cmd; 	// Command code
	can.cd.uc[1] = n; 	// Load 'n' (non-aligned)
	can.cd.uc[2] = n >> 8; 
	can.cd.uc[3] = n >> 16; 
	can.cd.uc[4] = n >> 24;
	sendcanmsg(&can);
	return;	
}
/******************************************************************************
 * void sendCMD_crc32(u32 pstart, u32 pend, u32 canid);
 * @brief 	: Send CAN msg to app to request unit to reply with crc32
 * @param	: pstart = address of first byte in stm32 memory
 * @param	: canid = CAN id
 * @param	: pend   = address of last byte+1 in stm32 memory
*******************************************************************************/
void sendCMD_crc32(u32 pstart, u32 pend, u32 canid)
{
	struct CANRCVBUF can;
	can.id = canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.dlc = 8;
	/* Size goes in uc[1] - uc[3] */
	can.cd.ui[0] = ((pend - pstart) << 8);	// Count of bytes [1] - [3]
	can.cd.ui[1] = pstart; // Start address [4] - [7]
	can.cd.uc[0] = LDR_CRC;
	sendcanmsg(&can);
printf("SEND CRC REQ: %08X %08X %08X\n",pstart, pend, pend-pstart);
	return;	
}
/******************************************************************************
 * void sendWR(struct CANRCVBUF* pcan);
 * @brief 	: Send a WRITE data payload
 * @param	: pcan = pointer to CANRCVBUF ready except for ID upper bits
*******************************************************************************/
void sendWR(struct CANRCVBUF* pcan)
{
//	pcan->id &= 0x0ffffffe; 
	pcan->id &= ~(CAN_EXTRID_MASK | 1); // This should be 0x0ffffffe
	pcan->id |=  (CAN_EXTRID_DATA_WR  << CAN_DATAID_SHIFT);
	sendcanmsg(pcan);
	return;	
}
/******************************************************************************
 * void sendRD(u32 count, u32 canid);
 * @brief 	: Send a READ data payload
 * @param	: Number of bytes to read-- 1-8
 * @param	: canid = CAN id
*******************************************************************************/
void sendRD(u32 count, u32 canid)
{
//	pcan->id &= 0x0ffffffe; 
	struct CANRCVBUF can;
	can.id = canid | (CAN_EXTRID_DATA_RD << CAN_DATAID_SHIFT);
	if (count > 8) count = 8;
	can.dlc = count;
	sendcanmsg(&can);
	return;	
}
/******************************************************************************
 * void sendMASTERRESET_squelch(u32 waitct, u32 canid);
 * @brief 	: Send a command to stop the other units from sending CAN msgs
 * @param	: canid = CAN id
*******************************************************************************/
void sendMASTERRESET_squelch(u32 waitct, u32 canid)
{
	struct CANRCVBUF can;
	can.id = CAN_SQUELCH; 	// High priority msg
	can.dlc = 8;		// Payload size
	can.cd.ui[0] = waitct; 	// Tick count for units to squelch sending
	// CAN ID in payload causes other units to stop sending
	can.cd.ui[1] = canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	sendcanmsg(&can);	// Send msg 
printf("SQL # %d: %08x %d %d %08X\n",SendMsgCtr, can.id, can.dlc, can.cd.ui[0],can.cd.ui[1]);
	return;	
}
