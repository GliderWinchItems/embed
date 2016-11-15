/*******************************************************************************
* File Name          : can_write.c
* Date First Issued  : 05/20/2014
* Board              : PC
* Description        : Convert and output SD card packets to CAN ascii/hex format msgs
*******************************************************************************/
#include "common.h"


#include "/home/deh/svn_discoveryf4/common_all/trunk/USB_PC_gateway.h"
#include "PC_gateway_comm.h"
#include "can_write.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/unistd.h>

static unsigned char sequence;
/*******************************************************************************
 * void can_write(FILE* fpOut, struct PKTP *pp);
 * @brief 	: Convert packet to CAN ascii/hex msg
 * @param	: pp--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/

void can_write(FILE* fpOut, struct PKTP *pp)
{
	struct CANRCVTIMBUF can;
	struct PCTOGATEWAY pctogateway;

	/* Put packet into CAN struct */
	packet_convert(&can, pp);	// Copy the fields

	/* Convert binary CAN struct into output format. */
	pctogateway.cmprs.seq = sequence++;  	// Add sequence number (to make 'cangate' happy)
	pctogateway.mode_link = 2;		// Gonzaga mode for gateway format
	USB_toPC_msg_mode(fileno(fpOut), &pctogateway, &can.R);	// Setup and 'write'

	return;
}
