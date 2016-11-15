/******************************************************************************
* File Name          : USB_PC_gateway.h
* Date First Issued  : 10/16/2013
* Board              : Discovery F4
* Description        : PC<->gateway 
*******************************************************************************/

#ifndef __USB_PC_GATEWAY
#define __USB_PC_GATEWAY

#include "common_misc.h"
#include "common_can.h"

/* **************************************************************************************/
int USB_PC_get_msg_mode(int fd, struct PCTOGATEWAY* ptr, struct CANRCVBUF* pcan);
/* @brief	: Build message from PC, various modes modes
 * @param	: fd = file descriptor
 * @param	: ptr = Pointer to msg buffer (see common_can.h)
 *              : ptr->mode_link = selection of PC<->gateway mode and format (binary, ascii,...)
 *              : return: ptr->c[] = binary msg, possibly compressed
 *              : return: ptr->asc[] = asc line
 *              : return: ptr->ct; ptr->ctasc; counts for above, repsectively.
 *              : return: ptr->seq; sequence number extracted., (if applicable to mode)
 * @param	: pcan = pointer to struct with CAN msg (stm32 register format)
 * @return	: Low order 16 bits = msg completion coded
 *		:  1 = completed; ptr->ct hold byte count
 *              :  0 = msg not ready; 
 *              : -1 = completed, but bad checksum
 *  		: -2 = completed, but too few bytes to be a valid CAN msg
 *              : -3 = bad mode selection code
 *              : -4 = 'read' returned an error
 *		: Hi order 16 bits = compression return codes (CANuncompress)
 * ************************************************************************************** */
int USB_toPC_msgASCII(int fd, struct PCTOGATECOMPRESSED* p);
/* @brief	: Send msg to PC after converting binary msg to ASCII/HEX 
 * @param	: fd = file descriptor
 * @param	: p = Pointer to struct with bytes to send to PC
 * @return	: count of bytes written
 * ************************************************************************************** */
int USB_toPC_msgBIN(int fd, struct PCTOGATECOMPRESSED* p);
/* @brief	: Send msg to PC in the binary format
 * @brief	: fd = file descriptor
 * @param	: p = Pointer to struct with bytes to send to PC
 * @return	: count of bytes written
 * ************************************************************************************** */
int USB_toPC_msg_mode(int fd, struct PCTOGATEWAY* ptr, struct CANRCVBUF* pcan);
/* @brief	: Send msg to PC in selected mode
 * @param	: fd = file descriptor
 * @param	: pcan = pointer to struct with CAN msg (stm32 register format)
 * @param	: ptr = Pointer to msg buffer
 *              : ptr->mode_link = selection of PC<->gateway mode and format (binary, ascii,...)
 *              : ptr->mode_send = CAN msg (from calling routine) is binary or ascii
 *              : ptr->c[] = binary msg, possibly compressed
 *              : ptr->asc[] = asc line
 *              : ptr->ct; ptr->ctasc; counts for above, repsectively.
 *              : ptr->seq; sequence number to be sent., (if applicable to mode)
 * @return	: postive = number of bytes written
 *              : negative = error
 *              : -1 = The bozo that called this routine gave us booogus ptr->mode_link|send!
 * ************************************************************************************** */
int USB_toPC_msg_asciican(int fd, char* pin, struct PCTOGATEWAY* ptr);
/* @brief	: I have a CAN msg in ASCII/HEX (no seq, no chksum).  Send in selected mode.
 * @param	: fd = file descriptor
 * @param       : pin = Pointer to ascii msg ('\0' or '\n' terminator)
 * @param	: ptr = Pointer to struct holding many things:
 *              : ptr->mode_link = selection of PC<->gateway mode and format (binary, ascii,...)
 *              : ptr->seq; sequence number to be sent., (if applicable to mode)
 * @return	: postive = number of bytes written
 *              : negative = error
 *              : -1 = The bozo that called this routine gave us booogus ptr->mode_link!
 * @NOTE	: Be sure ptr->mode_link is set!
 * ************************************************************************************** */


#endif 

