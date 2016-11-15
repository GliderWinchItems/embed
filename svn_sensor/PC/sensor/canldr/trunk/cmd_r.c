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

/******************************************************************************
 * int cmd_r_init(char* p);
 * @brief 	: Send RESET CAN msg to gateway
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/

int cmd_r_init(char* p)
{
	struct CANRCVBUF can;
	
	printf("Sending high priority RESET command\n");
	can.id = CAN_RESETALL | CAN_DATAID_MASTERRESET ;	// RESET msg id
	can.dlc = 0;		// No data payload

// DEBUG STUFF
//can.id = (CAN_UNITID_SE4   | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT)); can.dlc = 1; can.cd.u8[0]=LDR_RESET; // One unit only RESET
//can.id = (CAN_UNITID_OLI2  | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT)); can.dlc = 1; can.cd.u8[0]=LDR_RESET; // One unit only RESET
printf("%08x\n",can.id);


	CANsendmsg(&can);	// Send it to the output

	return 0;
}



