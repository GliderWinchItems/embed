/******************************************************************************
* File Name          : can_gps_phasing.h
* Date First Issued  : 06/03/2015
* Board              : F01 F4
* Description        : Phasing systick clocking with GPS CAN1 msgs
*******************************************************************************/
/* 
See p610 of Ref Manual 
*/

#ifndef __CAN_GPS_PHASING
#define __CAN_GPS_PHASING
#include "../../../../svn_common/trunk/common_can.h"

/* Used to reduce the accumulation of error in intervals scale deviation to "whole.fraction" form. */
#define TIMESCALE	16	// Number of bits to scale deviation of clock upwards

/* Averaging the ticks per 1/64th sec */
#define TICKAVEBITS	7	// Number bits in averaging size
#define TICKAVESIZE	(1 << TICKAVEBITS)	// Use this for scaling

/*******************************************************************************/
void can_gps_phasing_init(struct CAN_CTLBLOCK* pctl);
/* @brief	: Initializes SYSTICK and low level interrupts
 * @param	: pctl = pointer to CAN control block 
********************************************************************************/

/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
// These hold the address of the function that will be called
extern void 	(*highpriority_ptr)(void);		// FIFO 1 -> I2C1_EV  (low priority)
extern void 	(*lowpriority_ptr)(void);		// FIFO 0 -> I2C1_ER  (low priority
extern void 	(*systickLOpriority_ptr)(void);		// SYSTICK -> IC2C2_EV (low priority)
extern void 	(*systickHIpriority_ptr)(void);		// SYSTICK handler (very high priority) (2048/sec)
extern void 	(*systickHIpriorityX_ptr)(void);	// SYSTICK handler (very high priority) (64/sec)
extern void 	(*fifo1veryLOpriority_ptr)(void);	// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_ER (very low priority)

/* Deal with deviation from ideal */
extern u32	ticks64thideal;		// Ideal number of ticks in 1/64th sec

/* "whole.fraction" amounts for adjusting ticks in each SYSTICK interval (64/sec). */
extern s32	deviation_one64th;	// SCALED ticks in 1/64th sec actual

extern volatile u32	stk_64flgctr;	// 1/64th sec demarc counter/flag
extern u32 	systicktime;		// Used for speed computations (or other end-of-1/64th messing around)
extern s32	stk_32ctr;		// 1/2048th tick within 1/64th sec counter

/* Pointer for calling another routine for extending RX0,1 interrupt processing. */
extern void (*can_gps_phasing_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk); 


#endif 

