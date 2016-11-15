/******************************************************************************
* File Name          : cmd_s.c
* Date First Issued  : 12/10/2013
* Board              : PC
* Description        : Send test msgs to CAN gateway
*******************************************************************************/
/*
03/10/2015 rev 297 revised s command to send burst of one CAN id based on some other CAN msg poll
*/

#include "cmd_s.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

#define S_CMD_BUFSIZE	256
static int cmd_s_initsw = 0;		// 0 = not intialized
static char cmd_s_buf[S_CMD_BUFSIZE];	// File input buffer
//static int cmd_s_tim_b_msgs;		// Time between each msg 
//static int cmd_s_tim_b_filerepeat;	// Time between before repeating file sequence

//static struct timeval tim_day;
//static struct timezone tz;

static u8 canseqnumber = 0;
extern int s_cmd_onoff;

static struct CANRCVBUF can;
static struct CANRCVBUF can2;
static struct CANRCVBUF can_poll;
static u32 numpolls;
static u32 burstct;

/******************************************************************************
 * static int cmd_s_init(FILE* fpList);
 * @brief 	: Setup timing
 * @param	: fpList = file with test msgs
*******************************************************************************/
static int cmd_s_init(FILE* fpList)
{
	if (cmd_s_initsw == 1) return 1;

	rewind(fpList);
	/* Setup CAN msg to send from data on input file line. */
	while ( (fgets (&cmd_s_buf[0],S_CMD_BUFSIZE, fpList)) != NULL)	// Get a line
	{
		if (strncmp(cmd_s_buf, "//C",3) == 0) // Select the line that has data.
		{
			sscanf (&cmd_s_buf[3],"%x %x %llx %x %x %d %d",&can.id, &can.dlc, &can.cd.ull, &can_poll.id, &can2, &numpolls, &burstct);

can2.dlc = 2;

printf("COMMAND S CAN msg input line: %s",&cmd_s_buf[3]);
printf("COMMAND S CAN msg input: ID: %08X DLC: %x payload: ", can.id, can.dlc);
int i;
for (i = 0; i < can.dlc; i++) printf("%02X ",can.cd.uc[i]);
printf(" Poll ID    : %08X\n Response ID: %08X\n Numpolls : %d\n Burstct  : %d\n",can_poll.id, can2.id, numpolls, burstct);
printf("\n");
			cmd_s_initsw = 1;
			return 0 ;
		}
	}
	return -1;	// End of file

}
/******************************************************************************
 * void cmd_s_do_msg(FILE* fpList, int fd, struct CANRCVBUF);
 * @brief 	: Send CAN msgs
 * @param	: fpList = file with test msgs
 * @param	: fd = file descriptor for serial port
*******************************************************************************/

static int throttle = 0;
void cmd_s_do_msg(FILE* fpList, int fd, struct CANRCVBUF* pcan)
{
	struct PCTOGATEWAY pctogateway; 
//	struct timeval tim_now;
	int i;
	
	if (s_cmd_onoff == 0 ) return;	// Sending not enabled
	if (cmd_s_initsw < 0) return;	// Init failed
	if (cmd_s_initsw == 0) cmd_s_init(fpList); // Take care of init

	if (pcan->id == can_poll.id)
	{
		throttle += 1;
		if (throttle >= numpolls)	
		{ // Time msg used to throttle output		
			throttle = 0;
//printf("S COMMAND %08X %llx %08X\n", can.id, can.cd.ull, can_poll.id);
			for (i = 0; i < burstct; i++)
			{
can.cd.ull += 1;
				pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
				pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
				USB_toPC_msg_mode(fd, &pctogateway, &can); 	// Send to file descriptor (e.g. serial port)
			}
		}
pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
can_poll.dlc = 1;
USB_toPC_msg_mode(fd, &pctogateway, &can2); 	// Send to file descriptor (e.g. serial port)
	}
	

	return;
}



