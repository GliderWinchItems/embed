/******************************************************************************
* File Name          : common_cmd.h
* Date First Issued  : 11/22/2014
* Board              : PC
* Description        : Routines common to many 'cmd' functions
*******************************************************************************/

#ifndef __COMMON_CMD
#define __COMMON_CMD

/******************************************************************************/
void sendcanmsg(struct CANRCVBUF* pcan);
/* @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
void sendCMD_simple(u8 cmd, u32 canid);
/* @brief 	: Send a command with only the command code payload byte
 * @param	: Command code (see: common_can.h)
 * @param	: canid = CAN id
*******************************************************************************/
void sendCMD_25(u8 cmd, u32 canid, u32 n);
/* @brief 	: Send a command with command code payload byte + a 4 byte number
 * @param	: Command code (see: common_can.h)
 * @param	: canid = CAN id
 * @param	: 4 bytes that goes into payload bytes 2-5, (little endian)
*******************************************************************************/
void sendCMD_crc32(u32 pstart, u32 pend, u32 canid);
/* @brief 	: Send CAN msg to app to request unit to reply with crc32
 * @param	: pstart = address of first byte in stm32 memory
 * @param	: canid = CAN id
 * @param	: pend   = address of last byte+1 in stm32 memory
*******************************************************************************/
void sendWR(struct CANRCVBUF* pcan);
/* @brief 	: Send a WRITE data payload
 * @param	: pcan = pointer to CANRCVBUF ready except for ID upper bits
*******************************************************************************/
void sendRD(u32 count, u32 canid);
/* @brief 	: Send a READ data payload
 * @param	: Number of bytes to read-- 1-8
 * @param	: canid = CAN id
*******************************************************************************/
void sendMASTERRESET_squelch(u32 waitct, u32 canid);
/* @brief 	: Send a command to stop the other units from sending CAN msgs
 * @param	: canid = CAN id
*******************************************************************************/

#endif
