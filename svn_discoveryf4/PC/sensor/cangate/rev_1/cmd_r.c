/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_r.c
* Author	     : deh
* Date First Issued  : 09/22/2013
* Board              : PC
* Description        : Send master reset
*******************************************************************************/
/*
*/

#include "cmd_r.h"
#include "gatecomm.h"
extern int fdp;	/* port file descriptor */
static 	u32 keybrd_id;

static u8 canseqnumber = 0;

static void sendcanmsg(struct CANRCVBUF* pcan)
{
	struct PCTOGATEWAY pctogateway; 
	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
printf("%08x %d %d\n",pcan->id, pcan->dlc, pcan->cd.u8[0]);
	return;
}


/******************************************************************************
 * int cmd_r_init(char* p);
 * @brief 	: Send RESET CAN msg to gateway
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/
int cmd_r_init(char* p)
{
	struct CANRCVBUF can;


	if (strlen(p) < 6)
	{ // Here too few chars
		printf("MASTER RESET resets all units\n");
		can.id 	= CAN_RESETALL | CAN_DATAID_MASTERRESET ;	// RESET msg id
		can.dlc = 0;		// No data payload
		sendcanmsg(&can);

		return -1;
	}
	
	sscanf( (p+1), "%x",&keybrd_id);
	printf ("ID: %x\n",keybrd_id);
	can.id 	= (keybrd_id   | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT)); 
	can.dlc = 1; 
	can.cd.u8[0] = LDR_RESET; // One unit only RESET
		

// DEBUG STUFF
//can.id = (CAN_UNITID_SE3   | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT)); can.dlc = 1; can.cd.u8[0]=LDR_RESET; // One unit only RESET
//can.id = (CAN_UNITID_OLI2  | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT)); can.dlc = 1; can.cd.u8[0]=LDR_RESET; // One unit only RESET


	sendcanmsg(&can);

	return 0;
}



