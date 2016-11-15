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

/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
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
		printf("Too few characters.  Enter the unit CAN ID, e.g.\nr 00600000\n");
		return -1;
	}
	
	sscanf( (p+1), "%x",&keybrd_id); // Get CAN ID to send
	can.id 	= keybrd_id; 
	can.dlc = 1; 
	can.cd.uc[0] = LDR_RESET;	// Code that should cause the unit to RESET
	printf ("This CAN ID will be sent: %x\n",can.id);
	printf ("  with payload size %d\n,   and RESET code %d\n", can.dlc, can.cd.uc[0]);

	sendcanmsg(&can);	// This should cause thel unit to do a software forced system RESET

	return 0;
}



