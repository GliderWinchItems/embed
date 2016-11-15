/******************************************************************************
* File Name          : launchparm_xfr.c
* Date First Issued  : 11/14/2016
* Board              : 
* Description        : Transfer of launch parameters
*******************************************************************************/
#include "lauchparm_xfr.h"

static uint16_t state = 0; 	// Launch parameter sequence state
static uint16_t error_code_to;	// Return code in case of a timeout
static struct CANRCVBUF lpcan;	// Last CAN msg we sent

/*******************************************************************************
 * static void quitting(int x);
 * @brief	: Take of some things common to all exits
 * @param	: x = error code array index
********************************************************************************/
static void quitting(int x)
{
	p->launcherror[x] += 1;		// Running count of this error
	p->launchparm_timer_flg = 0; 	// Quitting sequence, so turn timer off
	state = 0;	
	return;
}
/*******************************************************************************
 * static void request_burst(struct MCFUNCTION* p);
 * @brief	: Take of some things common to all exits
 * @param	: x = error code array index
********************************************************************************/
static void request_burst(struct MCFUNCTION* p)
{
	p->launchparm_burst ctr = 0;	// Reset count msg in the burst

	lpcan.id = p->cancmdid;
	lpcan.dlc = 2;
	lpcan.cd.uc[0] = CMD_SEND_LAUNCH_PARM;
	lpcan.cd.uc[1] = p->launchparm_index;
	can_hub_send(&lpcan, p->phub_tension);// Send CAN msg to 'can_hub'
	return;
}
/*******************************************************************************
 * int lauchparm_xfr_poll(struct CANRCVBUF* pcan, struct MCFUNCTION* p, uint32_t* ptbl);
 * @brief	: Initializes RX msgs to check for reset
 * @param	: pctl = pointer to CAN control block 
 * @param	: p = pointer to struct with things such as command CAN id
 * @param	: ptbl = pointer to flat table for storing received four byte values
 * @return	:  2 = Sender has no parameters to send (parameter size = 0)
 *		:  1 = success
 *		:  0 = no further action needed;
 *		: -1 = dlc length of handshake response not the expected 3
 *		: -2 = Burst ct from PC is too large
 *		: -3 = PC's parameter list size not same as ours
 *		: -4 = Burst msg dlc is not the expected 6
 *		: -5 = Sender index is not same as ours
 *		: -6 = CRC check failure
********************************************************************************/
/*
The following is called from the polling loop.
If pcan is NULL there is no CAN msg, so the polling is for timeout checking.
*/
int lauchparm_xfr_poll(struct CANRCVBUF* pcan, struct MCFUNCTION* p, uint32_t* ptbl)
{


	/* Check if MC is making a request to transfer. */
	if (p->req_launchparm != 0)
	{ // Yes, setup handshake CAN msg and put in CAN queue
		state = 0;	// Reset sequence to beginning
		p->req_launchparm = 0;	// Reset flag
		/* Setup and send handshake msg to PC. */		
		lpcan.id = p->cancmdid;
		lpcan.dlc = 2;
		lpcan.cd.uc[0] = CMD_REQ_LAUNCH_PARM;
		lpcan.cd.uc[1] = p->launchparm_burstct;
		lpcan.cd.uc[2] = p->launchparm_size;
		lpcan_hub_send(&can, p->phub_mc);// Send CAN msg to 'can_hub'
		p->hbct_ticks = (p->ten_a.hbct * tim3_ten2_rate) / 1000; // Convert ms to timer ticks
		// Set next DTWTIME timeout count
		p->launchparm_timeout = DTWTIME + p->hbct_ticks * LAUNCH_PARAM_RETRY_TIMEOUT;
		p->launchparm_timer_flg = 1;	// Show timeout period begins
	}

	/* Check for a timeout */
	if (p->launchparm_timer_flg != 0)
	{ // Here there is a timeout underway
		if ((int(p->launchparm_timeout) - int(DTWTIME)) < 0)
		{ // Here, there was a timeout
			p->launchparm_retry_ctr += 1;
			if (p->launchparm_retry_ctr >= LAUNCH_PARAM_RETRY_CT)
			{ // Here, retry by sending last msg.
				can_hub_send(&lpcan, p->phub_mc);// Send CAN msg to 'can_hub'
				return 0;
			}
			else
			{ // Timed out there are no more retires.
				p->launchparm_timer_flg = 0; // Quit timing
				state = 0;	// Reset sequence state
				return error_code_to;	// Return time out code in case of a timeout
			}
		}
	}

	/* Return if there is no CAN msg to handle */
	if (pcan == NULL) return 0;

	/* Is this CAN msg a command for us? */
	if (pcan->id != p->a.canid) return 0;

	/* Deal with the CAN msg */
	switch (state)
	{
	case 0: // Expecting a response to handshake msg we sent

		// Is this command msg carry the code we expect?
		if (pcan->cd.uc[0] != CANID_CMD_MC) return 0; // Command for something else

		// We have a response, so disable timer
		p->launchparm_timer_flg != 0;
		
		// Is the length correct?
		if (pcan->dlc != 3)
		{ // No.  Probably a programming error.
			p->launcherror[0] += 1;	// Running count of this error
			quitting(0);
		 	return -1;		
		}

		// Is the sender going to send too many msgs in the burst?
		if (pcan->cd.uc[1] > p->launchparm_burstct)
		{ // Sender's burst size too big: they should have used ours!
			quitting(1);
		 	return -2;
		}
		p->launchparm_burstsize = pcan->cd.uc[1];  // Use the sender's size

		// Is sender telling us parameters are not available?
		if (pcan->cd.uc[2] == 0) return 2;

		// Is parameter size the same?
		if (pcan->cd.uc[2] != p->launchparm_size)
		{
			quitting(2);
		 	return -3;					
		}
		p->launchparm_index = 0;

		/* At this point agreement on burst count and parameter size */

	case 1:	// Send request for a burst
		request_burst();	// Setup and send CAN msg for next burst

	case 2: // Set next DTWTIME timeout count
		// Restart timer	
		p->launchparm_timeout = DTWTIME + p->hbct_ticks * LAUNCH_PARAM_RETRY_TIMEOUT;
		p->launchparm_timer_flg = 1;

		state = 3;

	case 3: // Here, we received command msg for us
		// Does this command msg carry the code we expect?
		if (pcan->cd.uc[0] != CANID_CMD_MC) return 0; // A command is for something else
		
		// Is the length correct?
		if (pcan->dlc != 6)
		{
			quitting(4);
		 	return -4;		
		}

		// Is index the same as ours?
		if (pcan->cd.uc[1] != p->launchparm_index)
		{
			quitting(5);
		 	return -5;		
		}

		/* Here, msg length OK, and index matches. */

		// Extract from payload & store the parameter in a flat table
		*ptbl(p->launchparm_index)  = pcan.cd.uc[2] <<  0;
		*ptbl(p->launchparm_index) |= pcan.cd.uc[2] <<  8;
		*ptbl(p->launchparm_index) |= pcan.cd.uc[2] << 16;
		*ptbl(p->launchparm_index) |= pcan.cd.uc[2] << 24;
		
		// Check if end-of-list
		p->launchparm_index += 1;	// Advance index
		if (p->launchparm_index > p->launchparm_size)
		{ // At end.  Compute and check CRC
			state = 0;
			// Do check
			quitting(7);// #### need to fix for error array ####
			return 1;	// Success		
			// Else
			quitting(6);	// CRC error
		 	return -6;	
		}

		// Check if end of burst
		p->launchparm_burst_ctr += 1;
		if (p->launchparm_burst_ctr >= p->launchparm_burstsize)
		{ // End of burst, so request another
			request_burst();	// Setup and send CAN msg for next burst
			state = 2;
		}
		else
		{ // Expecting more msgs in current burst
			// New timer timeout
			p->launchparm_timeout = DTWTIME + p->hbct_ticks * LAUNCH_PARAM_RETRY_TIMEOUT;
			p->launchparm_timer_flg = 1;
			state = 3;
		}
	}	
	return 0;
}
