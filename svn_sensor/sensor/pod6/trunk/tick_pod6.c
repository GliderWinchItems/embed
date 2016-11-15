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
that pointer to point to the "Continuation...' routine that does the remainder of the setup.
but at a priority that is higher than the polling loop of 'main', but lower that the
CAN and other time dependent operations.

*/

#include <stdlib.h>
#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "pinconfig.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "canwinch_pod_common_systick2048.h"
#include "pod6.h"
#include "DTW_counter.h"

static const u32 cantbl[] = { 
0x0f200000,\
0x0f200000,\
0x0f200000,\
0x0f200000,\
0x0f200000,\
0x0f200000,\
0x0f200000,\
0x0f200000,\
\
0x0e200000,\
0x0f200000,\
0x0e200000,\
0x0f200000,\
0x0e200000,\
0x0f200000,\
0x0e200000,\
0x0f200000,\
\
0x0c200000,\
0x0f200000,\
0x0e200000,\
0x0c200000,\
0x0f200000,\
0x0e200000,\
0x0f200000,\
0x0e200000,\
\
0x0c200000,\
0x0a200000,\
0x0f200000,\
0x0a200000,\
0x0f200000,\
0x0a200000,\
0x0a200000,\
0x0a200000,\

0x0a200000,\
0x0f200000,\
0x0a200000,\
0x0a200000,\
0x0f200000,\
0x0f200000,\
0x0f200000,\
0x0f200000,\
\
0x0e200000,\
0x0f200000,\
0x0e200000,\
0x0f200000,\
0x0e200000,\
0x0f200000,\
0x0e200000,\
0x0f200000,\
\
0x0c200000,\
0x0f200000,\
0x0e200000,\
0x0c200000,\
0x0f200000,\
0x0e200000,\
0x0c200000,\
0x0f200000,\
\
0x0a200000,\
0x0a200000,\
0x0a200000,\
0x0a200000,\
0x0a200000,\
};


/* ==== test ======= */
static struct CANRCVBUF can_msg_shaft;
static struct CANRCVBUF can_msg_speed;


/* Pointer to next routine if these are chained */
// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_EV (very low priority) -> Here -> Next (or return)
void 	(*Continuation_of_I2C2_EV_IRQHandler_ptr)(void) = 0;

/* Error counter */
u32	can_msgovrflow_6 = 0;		// Count times xmt buffer was full

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
can_msg_shaft.id = 0xffe00000;
	/* Set address to routine to handle end-of-interval */
	systickLOpriority_ptr = &Continuation_of_I2C2_EV_IRQHandler;
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
	can_msg_put_sys(p);

	return;
}
/*######################### WARNING UNDER INTERRUPT #####################################
 * Continuation of I2C2_EV_IRQHandler (which was triggered by SYSTICK_IRQHandler
 * Complete any measurement wrapup and send CAN msg to CO logger & others
 *####################################################################################### */
static u32	throttleLED;
u8	LEDflg = 0;
u32 dbgStk = 0;
u32 dbgSctr = 0;
u32 z1,z2;
int dbgTz;
u32 dbgTz_prev;
int dbgTz_max = 0;
int dbgTz_min = 0x7fffffff;
int ctrZ = 14;
#define QWSTART	100
int qw = QWSTART;
int qwct = 0;
u32 tmpid;
static int fid_ctr = 0;

void Continuation_of_I2C2_EV_IRQHandler(void)
{
	/* Coming into this routine 2048/sec */
	struct CANRCVBUF can;

if  ((stk_32ctr & 0x1f) == 0x11)
{
	dbgSctr += 1;
	if (dbgSctr >= 64)
	{
		dbgSctr = 0;
		dbgStk = 1;
	}
}

	/* ==== test ======= */
	if ((stk_32ctr & 0x1f) != 0) return;
	// Here, 64 per sec
	throttleLED += 1;
	if ((throttleLED & 0x3f) == 0)
	{
		LEDflg = 1;
	}

//qwct += 1;
//if (qwct >= 2*64) {qwct = 0; qw -= 2; if (qw < 1) qw = QWSTART;}

can_msg_shaft.cd.ull = 0;
tmpid = 0xff800000;

	can.id = 0x00200000; // Send fiducial
	can.dlc = 3;
	can.cd.ui[1] = fid_ctr;
	can.cd.us[0] = throttleLED;
	fid_ctr += 1;
	can_msg_put_ext_sys(&can, 3, 0);
	qw = 1;

int i;
int r;
for (i = 0; i < 100; i++)
{
	r = (rand() & 2047) << 21;
	if (r == 0x00200000) r = 0xffc00000;
//r = 0xaa400000;
	can_msg_shaft.id = r;
	can_msg_shaft.dlc = 8;
	can_msg_shaft.cd.us[0] = i;
//  	can_msg_shaft.cd.ui[1] = DTWTIME;

//	can_msg_shaft.cd.us[1] = xmit seq: filled in by CAN driver
//	can_msg_shaft.cd.ui[1] = DTW timer filled in by CAN driver
dbgTz_prev = DTWTIME;
	can_msg_put_ext_sys(&can_msg_shaft, 3, 0);
dbgTz = (int)((u32)DTWTIME - (u32)dbgTz_prev);

if (ctrZ++ >= 15){ctrZ = 0; dbgTz_max = 0; dbgTz_min = 0x7fffffff;}
if (dbgTz > dbgTz_max) dbgTz_max = dbgTz;
if (dbgTz < dbgTz_min) dbgTz_min = dbgTz;
qw += 1;
}
	/* Call other routines if an address is set up */
	if (Continuation_of_I2C2_EV_IRQHandler_ptr != 0)	// Having no address for the following is bad.
		(*Continuation_of_I2C2_EV_IRQHandler_ptr)();	// Go do something
	return;
}

