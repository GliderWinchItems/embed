/******************************************************************************
* File Name          : cmd_a.c
* Date First Issued  : 11/04/2014
* Board              : PC
* Description        : Same as M command but monitors ascii in CAN msgs
*******************************************************************************/
/*
*/

#include "cmd_a.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"


extern int fdp;	// file descriptor for serial port

static u32 canseqnumber;
#define CMDALINESIZE	256
static char buff[CMDALINESIZE];
static char* bptr = &buff[0];


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
	return;
}
/******************************************************************************
 * int cmd_a_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
static 	u32 keybrd_id;
static struct CANRCVBUF can;

int cmd_a_init(char* p)
{
	if (strlen(p) < 10)
	{ // Here too few chars
		printf("Too few chars for the 'a' command (needs 8), example\na 30e00000 [for a command and unit id]\n");
		return -1;
	}
	
	sscanf( (p+1), "%x",&keybrd_id);	// Extract CAN ID
	printf ("ID: %08X\n",keybrd_id);	// Hapless Op feedback
	
	/* Make a CAN ID that is a unit loader command */
//	can.id = (keybrd_id & ~(CAN_EXTRID_MASK | 1) ) | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.id = (keybrd_id & ~0x1);	
	can.dlc = 2;
	can.cd.uc[0] = LDR_ASCII_SW;

	/* Send a msg to start the unit sending 'printf' output in CAN msgs. */
	can.cd.uc[1] = 0xA5;	// Enable unit sending ascii msgs
	sendcanmsg(&can);	// Send set switch command

	bptr = &buff[0];	// Start output line at beginning

	return 0;
}
/******************************************************************************
 * void cmd_a_do_stop(void);
 * @brief 	; Send CAN msg to disable unit sending CAN ascii msgs
*******************************************************************************/
void cmd_a_do_stop(void)
{
printf("Hello?\n");
	/* Did we previously send a msg to enable it? */
	if (can.cd.uc[1] == 0)	{printf("NO\n");return;	}// No.

	/* Send a msg to stop the unit sending 'printf' output in CAN msgs. */
	can.cd.uc[1] = 0x0;	// Disable unit sending ascii msgs
	sendcanmsg(&can);	// Send set switch command
printf("Disable CAN sending ascii\n");
}

/******************************************************************************
 * void cmd_a_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output ascii payloads for the id that was entered with the 'a' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'a' has been
turned on by the hapless Op typing 'a' as the first char and hitting return.
*/
static int linect = 0;
//static int xct = 0;
void cmd_a_do_msg(struct CANRCVBUF* pcan)
{
	int i;
	if ( (pcan->id != can.id) || (pcan->cd.uc[0] != LDR_ASCII_DAT) ) return;

	if (pcan->dlc > 8) {printf("Bogus dlc: %d\n", pcan->dlc); return;}

	for (i = 1; i < pcan->dlc; i++)
	{
		if (pcan->cd.uc[i] == '\n') linect = 0;
		printf("%c",pcan->cd.uc[i]);

		/* In case '\n' is missing do a newline */
		linect += 1;
		if (linect >= 128)
		{
			linect = 0;
			printf("\n");
		}
	}
	return;
	
}

