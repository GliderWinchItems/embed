/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : can_log.h
* Hackerees          : deh
* Date First Issued  : 01/24/2013
* Board              : Olimex co board
* Description        : Post CAN interrupt stuff
*******************************************************************************/
/* 

*/

#ifndef __CAN_LOG
#define __CAN_LOG

#include "common.h"	// Such as 'u32'
#include "common_can.h"	// CAN ID|mask layout and structs

/******************************************************************************/
void can_log_init(void);
/* @brief 	: A bit of initialization
*******************************************************************************
 * 'sdlog log' loops waiting for operation to complete.  This can take many milliseconds, e.g.
 *  we have seen 254 ms worst-case with a Samsung 8 GB class 4 card.  Normally writes can take 
 *  15 or less ms, and average around 6 ms.  To provide for the rare big delay the message buffer
 *  is set to be large.
 */
int can_log_can(void);
/* @brief 	: Polling to write any available buffers to SD card
 * @return	: zero = OK.  negative = a non-recoverable error.
*******************************************************************************/
struct CANRCVSTAMPEDBUF* monitor_m_canbuf_get(void);
/* @brief 	: See if buffer with CAN msg is ready for sending to PC
 * @return	: pointer = 0 for data not ready, otherwise points buffer
*******************************************************************************/
u32 monitor_n_canbuf_get(void);
/* @brief 	: See if buffer with CAN id is ready for sending to PC
 * @return	: 0 = none, otherwise holds ID (ID == 0 not assigned)
*******************************************************************************/
void can_log_trigger_logging(void);
/* @brief 	: Set the low level interrupt to execute sd card write
*******************************************************************************/
void noncanlog_add(u8 *p, u8 ct);
/* @brief 	: Add a non-CAN item to be buffered for logging (call from mainline only)
 * @param	: p = pointer to data to be buffered
 * @param	: ct = number of bytes in data to be buffered
*******************************************************************************/

/*####################################################################################### */
void can_log_puttime(struct CANRCVBUF *p, unsigned long long ull);	
/* brief Set up CAN time sync msg for logging. Buffer once per sec msg with time
 * Enter here under a high priority interrupt from Tim4_pod_se.c
 * param	: p = pointer to time sync msg (64/sec)
 * param	: ull = system linux time (<<6)
 *####################################################################################### */

/* Running count of CAN msgs written to SD card */
extern volatile u32 canlogct;
extern volatile u32 canlogctr;

/* Pointer to functions to be executed under a low priority interrupt */
extern void 	(*logpriority_ptr)(void);	// Address of function to call forced interrupt

/* Unrecoverable SD write error */
extern int sd_error;


#endif 



