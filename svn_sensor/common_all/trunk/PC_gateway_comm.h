/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : PC_gateway_comm.h
* Author             : 
* Date First Issued  : 10/04/2013
* Board              : Not specific to PC or stm32
* Description        : PC<->gateway 
*******************************************************************************/

#ifndef __PC_GATEWAY_COMM
#define __PC_GATEWAY_COMM

#include "common_can.h"

/* **************************************************************************************/
int PC_msg_get(struct PCTOGATEWAY* ptr, u8 c);
/* @brief	: Build message from incoming bytes
 * @param	: ptr = Pointer to msg buffer (see common_can.h)
 * @param	: c = byte to add msg being built
 * @return	:  1 = completed; ptr->ct hold byte count
 *              :  0 = msg not ready; 
 *              : -1 = completed, but bad checksum
 *  		: -2 = completed, but too few bytes to be a valid CAN msg
 * ************************************************************************************** */
void PC_msg_initg(struct PCTOGATEWAY* p);
/* @brief	: Initialize struct for building a gateway message from the PC
 * @param	: Pointer to gatewau message wrapper (see common_can.h)
 * ************************************************************************************** */
void PC_msg_init(struct CANPCWRAP* p);
/* @brief	: Initialize struct for building a CAN message from the PC
 * @param	: Pointer to CAN message wrapper (see common_can.h)
 * ************************************************************************************** */
int PC_msg_prep(u8* pout, int outsize, u8* pin, int ct);
/* @brief	: Convert input bytes into output with byte stuffing, checksum and framing
 * @param	: pout = pointer to bytes with stuffing and chksum added 
 * @param	: outsize = size of output buffer (to prevent overflow if too small)
 * @param	: pin = Pointer to bytes to send
 * @param	: ct = byte count to send (does not include frame bytes, chksum, or stuffing bytes)
 * @return	: number of bytes in prepped message
 * ************************************************************************************** */
u8 CANgenchksum(u8* p, int ct);
/* @brief	: Generate a one byte checksum
 * @param	: p = pointer to array to be checksummed
 * @param	: ct = number of bytes to be checksumned
 * @return	: Checksum
 * ************************************************************************************** */



/* **************************************************************************************/
int USART1_PC_msg_get(struct PCTOGATEWAY* ptr);
/* @brief	: Build message from PC
 * @param	: ptr = Pointer to msg buffer (see common_can.h)
 * @return	: 0 = msg not ready; 1 = completed; ptr->ct hold byte count
 * ************************************************************************************** */
void USART1_PC_msg_init(struct CANPCWRAP* p);
/* @brief	: Initialize struct for building a CAN message from the PC
 * @param	: Pointer to CAN message wrapper (see common_can.h)
 * ************************************************************************************** */
void USART1_PC_msg_initg(struct PCTOGATEWAY* p);
/* @brief	: Initialize struct for building a gateway message from the PC
 * @param	: Pointer to gatewau message wrapper (see common_can.h)
 * ************************************************************************************** */
void USART1_toPC_msg(u8* ps, int size);
/* @brief	: Send msg to PC in binary with framing and byte stuffing 
 * @param	: ps = Pointer to bytes to send to PC
 * @param	: size = byte count to send (does not include frame bytes, no stuffing bytes)
 * ************************************************************************************** */
void USART1_GATEtoPC_msg(u8* ps, int size);
/* @brief	: Send CAN msg to PC
 * @param	: Pointer to gate.c message
 * @param	: size = byte count to send (does not include frame bytes, no stuffing bytes)
 * ************************************************************************************** */
int CANcompress(struct PCTOGATECOMPRESSED* pout, struct CANRCVBUF* pin);
/* @brief	: Send Gateway msg to PC
 * @param	: pout = pointer to output w compressed msg
 * @param	: pin = pointer to input with uncompressed msg
 * @return	: 0 = compressed; 1 = input not compressible
 * ************************************************************************************** */
void CANuncompress(struct CANRCVBUF* pout,  struct PCTOGATECOMPRESSED* pin);
/* @brief	: Send Gateway msg to PC
 * @param	: pout = pointer to output w uncompressed msg
 * @param	: pin = pointer to input with compressed msg
 * @return	: none
 * ************************************************************************************** */
void CANcopyuncompressed(struct CANRCVBUF* pout,  struct PCTOGATEWAY* pin);
/* @brief	: Copy incoming byte message (uncompressed) to CAN struct msg
 * @param	: pout = pointer to output 
 * @param	: pin = pointer to input
 * @return	: none
 * ************************************************************************************** */
void CANcopycompressed(struct CANRCVBUF* pout,  struct PCTOGATEWAY* pin);
/* @brief	: Copy incoming byte message (compressed) to CAN struct msg
 * @param	: pout = pointer to output 
 * @param	: pin = pointer to input
 * @return	: none
 * ************************************************************************************** */
int CANmsgvalid(struct CANRCVBUF* p);
/* @brief	: Check that struct looks like a valid CAN msg
 * @param	: p = pointer to input
 * @return	: 0 - OK, not zero = bad
 * ************************************************************************************** */
u8 CANgenchkcompress(u8* p, int ct);
/* @brief	: Generate a one byte checksum
 * @param	: p = pointer to array to be checksummed
 * @param	: ct = number of bytes to be checksumned
 * @return	: Checksum
 * ************************************************************************************** */


#endif 

