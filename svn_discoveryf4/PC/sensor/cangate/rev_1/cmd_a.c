/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_a.c
* Author	     : deh
* Date First Issued  : 09/26/2013
* Board              : PC
* Description        : Loader sequence for one unit
*******************************************************************************/
/*
*/

#include "cmd_a.h"
#include "cmd_p.h"
#include "gatecomm.h"


static int cmd_a_sw;

/******************************************************************************
 * int cmd_a_init(char* p);
 * @brief 	: 
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/

int cmd_a_init(char* p)
{
	struct CANRCVBUF can;

	if (ldfilesopen_sw == 0)
	{
		printf("Execute 'p' command first to open the .bin & .srec files\n"); return -1;
	}
	
	cmd_a_sw = 0;	// Switch for sequencing

	printf("Sending unit-only RESET command\n");
	can.dlc = 0;		// No data payload

	can.id = (CAN_UNITID_SE4   | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT)); can.dlc = 1; can.cd.u8[0]=LDR_RESET; // One unit only RESET


	CANsendmsg(&can);	// Send it to the output

	return 0;
}
/******************************************************************************
 * void cmd_a_do_msg(struct CANRCVBUF* p);
 * @brief 	: 
*******************************************************************************/
/*
Each received msg from the gateway comes here if command 'a' is in effect.
*/
void cmd_a_do_msg(struct CANRCVBUF* p)
{


	return;
}
/******************************************************************************
 * void cmd_a_timeout(void);
 * @brief 	: 
*******************************************************************************/
void cmd_a_timeout(void)
{
	return;
}


