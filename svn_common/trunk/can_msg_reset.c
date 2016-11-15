/******************************************************************************
* File Name          : can_msg_reset.c
* Date First Issued  : 06/27/2015
* Board              : F4 F103 with can_driver
* Description        : CAN msg forces system reset
*******************************************************************************/
#include "can_driver.h"
#include "../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/stm32/f4/scb.h"

/* Declaration of the grand rountine. */
static void can_msg_reset_msg(void* pctl, struct CAN_POOLBLOCK* pblk);

void (*can_msg_reset_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk) = NULL;	// Pointer for extending this routine's interrupt processing

static u32 myunitid_local;
/*******************************************************************************
 * int can_msg_reset_init (struct CAN_CTLBLOCK* pctl, u32 canid);
 * @brief	: Initializes RX msgs to check for reset
 * @param	: pctl = pointer to CAN control block 
 * @param	: canid = CAN id used for reset msg to this unit
 * @return	: 0 = OK, -1 failed: RX ptr was not NULL
********************************************************************************/

int can_msg_reset_init (struct CAN_CTLBLOCK* pctl, u32 canid)
{
	myunitid_local = canid;		// Save unit id used for causing RESET

	// 'can_driver.c' 'CAN_RX_IRQHandler' will call to this address
	pctl->ptrs1.func_rx = (void*)&can_msg_reset_msg; // Callback address for CAN RX0 or RX1 handler

	return 0;
}
/*#######################################################################################
 * Callled from can_driver.c RX0 or RX1 IRQHandler
 *####################################################################################### */
static void can_msg_reset_msg(void* pctl, struct CAN_POOLBLOCK* pblk)
{
	__attribute__((__unused__)) void* x = pctl;	// Get rid of unused warning.

	/* Compare the CAN ID in the msg just received with the UNIT id, and execute a software
   	   forced system reset if they match. */
	if (pblk->can.id == myunitid_local)	// Msg for this unit?
	{ // Here, a loader command (data type) for our unitid.  Is this a forced reset to this unit only?
		if ( ((pblk->can.dlc & 0xf) == 1) && (pblk->can.cd.u8[0] == LDR_RESET) )
			SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	}

	/* Call other routines if an address is set up, (e.g. 'can_gps_phasing' that saves the
           DTW tick count stored  in CAN_POOLBLOCK). */
	if (can_msg_reset_ptr != NULL)	// Option here is to chain to other routines
		(*can_msg_reset_ptr)(pctl, pblk);	// Go do something else (in a timely manner!)

	return;	
}
