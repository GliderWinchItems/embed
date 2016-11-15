/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : packet_mgr.c
* Hackeroos          : deh
* Date First Issued  : 12/23/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for managing sensor packet buffers
*******************************************************************************/
/*
This routine "manages" the packet buffering.  Each CAN msg ID has a set of packet
buffers.  The buffers are circular and as data comes in the buffer is filled.  The
mainline loop polls for a buffer that is complete and waiting.

This routine adds a set of buffers each time it encounters a new CAN msg ID.  A table
holds the pointers and CAN id for each set of buffers that have been setup.  The
polling for logging cycles through the table checking for a packet to write.

Rev 68 has the table search approach, before changes to make a linked list

Linked list scheme--
Each control block has a pointer to the next block, and the last one points back to
the first.  

When a new ID is encountered memory is allocated for the control block and that control
block is initialized and inserted into the list so the CAN msg ID's are in ascending
order.  If all the sensors send at the same time data arrives in the order of the CAN
msg priority.  This means search the linked list should only require one step.  However,
it means the list needs to be ordered when it is setup.


*/

#include "packet_mgr.h"
#include "mymalloc.h"

static struct PKT_CTL_BLOCK * pctb_begin = 0;	// Point to first control block in the list.

void screwed(void) {while (1==1);}	// What else could we do?

/*******************************************************************************
 * void packet_mgr_add(struct CANRCVTIMBUF * pcan);
 * @brief 	: Buffer a reading
 * @param	: pcan--pointer to struct with: time, can ID, can data
 * @return	: None
*******************************************************************************/
void packet_mgr_add(struct CANRCVTIMBUF * pcan)
{
	struct PKT_CTL_BLOCK * pwrk;	// Working pointer
	struct PKT_CTL_BLOCK * pnew;	// Pointer to newly allocated control block w buffers

	if (pctb_begin == 0)
	{ // Here no entries in the list -- Should be a ONE-TIME ONLY path taken.
		/* Get memory for a control block with packet buffers */

// Is there a problem with the address not being aligned to a long long boundary?
		pctb_begin = (struct PKT_CTL_BLOCK *)mymalloc(sizeof (struct PKT_CTL_BLOCK));
		if ( pctb_begin == 0) {screwed(); return;}

		/* Address that points to next control block w buffers.  The last one is NULL (zero). */
		pctb_begin->forward = 0;

		/* Set id_can in this new control block */
		pctb_begin->id_can = pcan->R.id;
	
		/* Initialize some one-time things, then add the reading */
		packpkt_init(pctb_begin, pcan);
		
		return;
	}

	/* Here--there is at least one entry in the list */

	pwrk = pctb_begin; 	// Start at beginning of list

	/* Here search the list--(We want this loop to be fast, so no frills and extra stuff.) */
	while (1==1)	// Break loop when last block has been checked.
	{
		if (pwrk->id_can != pcan->R.id)	// Does control block ID match msg ID?
		{
			if (pwrk->forward == 0) break;// Break when there is no next block
			pwrk = pwrk->forward;	// Get address of next control block
		}
		else
		{ // Here we got a match
			packpkt_add( pwrk, pcan); // Add reading to packet buffer
			return;	// DONE
		}
	}

	/* Here we have gone through the entire list loop without finding a match.
	   Setup a new control block and a set of buffers. */

	/* Get memory for a control block with packet buffers */
// Is there a problem with the address not being aligned to a long long boundary?
	pnew = (struct PKT_CTL_BLOCK *)mymalloc(sizeof (struct PKT_CTL_BLOCK));
	if ( pnew == 0) {screwed(); return;}

	/* Here, 'pwrk' points to the last control block in the list */
	pwrk->forward = pnew;	// Make last block searched point to new blocked added.
	pnew->forward = 0;	// Make new block added the last in the list.
	
	/* Set the can_id in the new control block */
	pnew->id_can = pcan->R.id;

	/* Initialize some one-time things and add the reading */
	packpkt_init(pnew, pcan);

	return;
}
/*******************************************************************************
 * struct PKT_PTR packet_mgr_get_next(void);
 * @brief 	: Get a point & ct to a packet buffer that is complete
 * @param	: pb--pointer to control block
 * @return	: pointer & ct: NULL & 0 = No data ready
*******************************************************************************/
struct PKT_PTR packet_mgr_get_next(void)
{
	struct PKT_CTL_BLOCK * pwrk=pctb_begin;	// Working pointer starts at beginning of list.
	struct PKT_PTR pkp = {0,0};	// This holds the return values (pointer, count)
	
	/* Cycle through all the CAN ID control blocks for a waiting packet buffer */
	while (pwrk != 0)
	{
		pkp = packpkt_get(pwrk);	// Any packets waiting?
		if (pkp.ptr != 0) return pkp;	// If yes, return char pointer & byte count
		pwrk = pwrk->forward;		// Get address of next control block
	}

	/* Here we made one pass through the list without finding any waiting packets */
	return pkp;

}

