/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : canmsg_send.h
* Author             : deh
* Date First Issued  : 07/01/2013
* Board              : RxT6
* Description        : At 1/64th sec demarcation, setup can messages
*******************************************************************************/


#ifndef __CANMSG_SEND_SE1
#define __CANMSG_SEND_SE1

/*######################### WARNING UNDER INTERRUPT ##################################### */
void canmsg_send(struct CANRCVBUF * p, int data1, int data2);
/* @param	: p = pointer to message to send
 * @param	: data1 = 1st 4 bytes of payload
 * @param	: data2 = 2nd 4 bytes of payload
 * @brief 	: send CAN msg
 *####################################################################################### */

/* Error counter */
extern u32	can_msgovrflow;		// Count times xmt buffer was full

#endif 

