/******************************************************************************
* File Name          : sand1.c
* Date First Issued  : 03/21/2014
* Board              : PC
* Description        : CAN conversion between hub-server and Java Echo2
*                    :  Based upon--"hub-server-sock-data.c"
* *************************************************************************** */
#include <syslog.h>
#include "hub-server-sock.h"
#include "hub-server-queue.h"

#include <string.h>
#include <stdio.h>
#include "PC_gateway_comm.h"	// Common to PC and STM32
/******************************************************************************
Compile, look at directory, listen on 32124, connect to netbook directly--
./m sand1 && ls -l && ./sand1 0.0.0.0 32124 10.1.1.80 32123

Or,
./m sand1 && ls -l && echo "UP AND READY" && echo "CLIENTS CAN NOW CONNECT TO 32124" && ./sand1 0.0.0.0 32124 10.1.1.80 32123

To complete the startup, in another windows,
nc localhost 32124

Or, for a nice output run Java, "Echo2"

Or, both!
* *************************************************************************** */

int hsd_new_in_data(connect_t *in, int size)
{
	return in->is_server ? size : size;		/* Avoid warnings */
}
/* ************************************************************************** */
int hsd_new_in_out_pair(connect_t *in, connect_t *out, int size)
/* 
This routine is entered when there is one or more complete lines from the 
connect side.

'in' points to the control block--see 'hub-server-sock.h'
ptibs points to the beginning of the line.
size is the amount buffered, and *may include more than one line*.

*/
{
	int n;
	char* plocalbuf = in->ptibs;
	int retstatus;
	int temp;
	struct PCTOGATEWAY pctogateway;
	struct CANRCVBUF canrcvbuf;
	char vv[64];


	PC_msg_initg(&pctogateway);		// Initialize struct for a msg from gateway to PC
	pctogateway.mode_link = MODE_LINK;	// This is more for a reminder if different modes are used later.

	while (size > 0)		// There can be multiple lines in the buffer passed to us.
	{				// Assemble a CAN msg from the incoming bytes
		size -= 1;		// Count chars
		if ( (retstatus = PC_msg_getASCII(&pctogateway, *plocalbuf++)) != 0) // Did this char complete a msg?
		{ // Here, either a good line, or an error, such as too many or few chars, checksum err, or odd number char pairs
			if (retstatus >= 1)	// Did this result in a completed message, without error?
			{ // Yes. Unpack and move into the standard message struct
				temp = CANuncompress_G(&canrcvbuf, &pctogateway.cmprs); // Unpack into CANRCVBUF struct
				if (temp >= 0) // Did it unpack OK?
				{ // Good to go.
/* ============================== Here we have the binary from the ascii/hex input =============================================== */
/* ============================== This is where the custom code goes ============================================================= */
					if (canrcvbuf.id == 0x40800000)	
					{
						sprintf (vv,".CAN ID: 0x%08x\n",canrcvbuf.id);
							n = hsq_enqueue_chars(&out->oq, vv, strlen(vv) );
							if(n != strlen(vv))
								syslog(LOG_INFO, "hsd_new_in_out_pair -- enqueue err, %d/%d\n", n, size);
						float fMp =( (float)(canrcvbuf.cd.ui[0]) / 103.5 ) - 0.15 ;
						double fRpm = ((double)canrcvbuf.cd.si[1]) * 0.1;
						sprintf (vv," MANIFOLD PRESSURE: %4.1f   RPM: %5.1f\n",fMp,fRpm);
							n = hsq_enqueue_chars(&out->oq, vv, strlen(vv) );
							if(n != strlen(vv))
								syslog(LOG_INFO, "hsd_new_in_out_pair -- enqueue err, %d/%d\n", n, size);
					}
/* ================================================================================================================================ */
				}
				else
				{ // More for debugging that real production use
					printf("Unpack error %d\n",temp);
				}
				PC_msg_initg(&pctogateway);	// Initialize struct for a msg from gateway to PC	
			}
			else
			{ // More for debugging than real production use
				if (retstatus != 0)
					printf("Assemble line error: %d\n",retstatus);
					PC_msg_initg(&pctogateway);	// Initialize struct for a msg from gateway to PC
			}
		}
	}	
	
	return n;
}

