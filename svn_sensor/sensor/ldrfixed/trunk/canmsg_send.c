/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : canmsg_send.c
* Author             : deh
* Date First Issued  : 07/01/2013
* Board              : RxT6
* Description        : At 1/64th sec demarcation, setup can messages
*******************************************************************************/

/* 


*/


#include "common_can.h"
#include "canwinch_pod_common_systick2048.h"
#include "se1.h"



/* Error counter */
u32	can_msgovrflow = 0;		// Count times xmt buffer was full


/*######################### WARNING MAY BE UNDER INTERRUPT #####################################
 * void canmsg_send(struct CANRCVBUF * p, int data1, int data2);
 * @param	: p = pointer to message to send
 * @param	: data1 = 1st 4 bytes of payload
 * @param	: data2 = 2nd 4 bytes of payload
 * @brief 	: send CAN msg
 *####################################################################################### */
void canmsg_send(struct CANRCVBUF * p, int data1, int data2)
{
	p->cd.ui[0] = data1;
	p->cd.ui[1] = data2;

//	can_msg_rcv_compress(p);	// Set byte count: according to MSB
	can_msg_setsize_sys(p, 8);	// Set byte count: Fixed xmt

 	/* Setup CAN msg in output buffer/queue */
	if ( can_msg_put_sys(p) <= 0)
		can_msgovrflow += 1;

	return;
}

