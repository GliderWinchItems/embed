/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : tick_pod.c
* Hackerees          : deh
* Date First Issued  : 02/16/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Post SYSTICK interrupt handling
*******************************************************************************/

/* 
This routine is entered following a SYSTICK interrupt.  The SYSTICK interrupt signals
the end|start of a 1/64th sec interval.  This is the reference point for the measurements,
and as such the measurement is "closed out," calculations made, and a CAN messages send.

The SYSTICK interrupt handling is done at high priority, where the time duration for the 
next interval is adjusted/updated, followed by triggering a low priority interrupt.  The
low priority interrupt calls a function via a pointer.  The 'init' in this routine sets
that pointer to point to the routine that continues the remainder of the setup.
but at a priority that is higher than the polling loop of 'main', but lower that the
CAN and other time dependent operations.

*/


#include "common_can.h"
#include "canwinch_pod_common_systick2048.h"
#include "se1.h"

/* ==== test ======= */
static struct CANRCVBUF can_msg_shaft;
static struct CANRCVBUF can_msg_speed;


/* Pointer to next routine if these are chained */
// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_EV (very low priority) -> Here -> Next (or return)
void 	(*Continuation_of_I2C2_EV_IRQHandler_ptr)(void) = 0;

/* Error counter */
u32	can_msgovrflow = 0;		// Count times xmt buffer was full


/* Subroutine prototypes */
void Continuation_of_I2C2_EV_IRQHandler(void);

/******************************************************************************
 * void tick_pod6_init(void);
 * @brief 	: Enable preparation and CAN xmt of measurement
*******************************************************************************/
void tick_pod6_init(void)
{
	/* === test ==== */
	can_msg_shaft.id = IAMUNITNUMBER | (CAN_DATAID_POD6_SHAFT << CAN_DATAID_SHIFT);
	can_msg_speed.id = IAMUNITNUMBER | (CAN_DATAID_POD6_SPEED << CAN_DATAID_SHIFT);

	/* Set address to routine to handle end-of-interval */
	systickLOpriority2_ptr = &Continuation_of_I2C2_EV_IRQHandler;

	return;
}
/******************************************************************************
 * void tick_send(struct CANRCVBUF * p);
 * @param	: p = pointer to message to send
 * @brief 	: send CAN msg
*******************************************************************************/
void tick_send(struct CANRCVBUF * p)
{
	/* Prepare CAN msg */
//	can_msg_rcv_compress(p);	// Set byte count: according to MSB
	can_msg_setsize_sys(p, 8);	// Set byte count: Fixed xmt

 	/* Setup CAN msg in output buffer/queue */
	if ( can_msg_put_sys(p) <= 0)
		can_msgovrflow += 1;

	return;
}
/*######################### WARNING UNDER INTERRUPT #####################################
 * Continuation of I2C2_EV_IRQHandler (which was triggered by SYSTICK_IRQHandler
 * Complete any measurement wrapup and send CAN msg to CO logger & others
 *####################################################################################### */
static u32	throttleLED;
u8	LEDflg = 0;

void Continuation_of_I2C2_EV_IRQHandler(void)
{
	/* ==== test ======= */
	throttleLED += 1;
	if (throttleLED >= 64)
	{
		throttleLED = 0;
		LEDflg = 1;
	}
	/* Prepare & send CAN msg */
	tick_send(&can_msg_shaft);
	can_msg_shaft.cd.ui[0] += 1;			// Make the msg change for the next round

	/* Prepare & send CAN msg */
	tick_send(&can_msg_speed);
	can_msg_speed.cd.ui[0] += 1;			// Make the msg change for the next round
	/* ==== end test ======= */


	/* Call other routines if an address is set up */
	if (Continuation_of_I2C2_EV_IRQHandler_ptr != 0)	// Having no address for the following is bad.
		(*Continuation_of_I2C2_EV_IRQHandler_ptr)();	// Go do something
	return;
}

