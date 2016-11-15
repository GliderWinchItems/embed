/******************************************************************************
* File Name          : cmd_s.c
* Date First Issued  : 12/10/2013
* Board              : PC
* Description        : Send test msgs to CAN gateway
*******************************************************************************/
/*
*/

#include "cmd_s.h"
#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"

#define S_CMD_BUFSIZE	256
static int cmd_s_initsw = 0;		// 0 = not intialized
static char cmd_s_buf[S_CMD_BUFSIZE];	// File input buffer
static int cmd_s_tim_b_msgs;		// Time between each msg 
static int cmd_s_tim_b_filerepeat;	// Time between before repeating file sequence

static struct timeval tim_day;
static struct timezone tz;

static u8 canseqnumber = 0;
extern int s_cmd_onoff;



/******************************************************************************
 * static int cmd_s_init(FILE* fpList);
 * @brief 	: Setup timing
 * @param	: fpList = file with test msgs
*******************************************************************************/
static int cmd_s_init(FILE* fpList)
{
	char c;
	cmd_s_initsw = 1;	// Initialized

while ( (fgets (&cmd_s_buf[0],S_CMD_BUFSIZE, fpList)) != NULL) printf("%s",cmd_s_buf);
rewind(fpList);
	
	while ( (fgets (&cmd_s_buf[0],S_CMD_BUFSIZE, fpList)) != NULL)	// Get a line
	{
		if (cmd_s_buf[0] == 'T')
		{ // Here, first non-comment line, assumed to have the control numbers
			sscanf(cmd_s_buf,"%c %u %u",&c,&cmd_s_tim_b_msgs, &cmd_s_tim_b_filerepeat);
			printf("HERE YAll!--- cmd_s_init() times: time between msg = %i  time before file repeat = %i \n",cmd_s_tim_b_msgs, cmd_s_tim_b_filerepeat);
			gettimeofday(&tim_day, &tz);
			return 0;
		}
	}
	printf("\nA line in the test msg file starting with T was not found...in cmd_s_init()\n");
	cmd_s_initsw = -1;	
	return -1;

}
/******************************************************************************
 * void cmd_s_do_msg(FILE* fpList, int fd);
 * @brief 	: Send CAN msgs
 * @param	: fpList = file with test msgs
 * @param	: fd = file descriptor for serial port
*******************************************************************************/
static int cmd_s_getnextmsg(FILE* fpList, struct CANRCVBUF *pcan)
{
	char c;
	while ( (fgets (&cmd_s_buf[0],S_CMD_BUFSIZE, fpList)) != NULL)	// Get a line
	{
		if (cmd_s_buf[0] == 'C')
		{
			sscanf (cmd_s_buf,"%c %x %x %llx",&c, &pcan->id, &pcan->dlc, &pcan->cd.ull);
//printf("%s\n",cmd_s_buf);
			return 0 ;
		}
	}
	return -1;	// End of file
}
#define THROTTLECT	8	// Number of polls before sending (each incoming msg is one poll)
static int throttle = 0;
void cmd_s_do_msg(FILE* fpList, int fd)
{
	struct CANRCVBUF can;
	struct PCTOGATEWAY pctogateway; 
//	struct timeval tim_now;
	int ret;

	if (s_cmd_onoff == 0 ) return;	// Sending not enabled
	if (cmd_s_initsw < 0) return;	// Init failed
	if (cmd_s_initsw == 0) cmd_s_init(fpList); // Take care of init

	if (throttle++ > THROTTLECT)
	{ // Time to send next msg
		ret = cmd_s_getnextmsg(fpList, &can);		// Get next msg from file
		if (ret < 0)	
		{
			fseek(fpList,0L,SEEK_SET);	// Start over
			cmd_s_getnextmsg(fpList, &can);
		}
		pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
		pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
		USB_toPC_msg_mode(fd, &pctogateway, &can); 	// Send to file descriptor (e.g. serial port)

		throttle = 0;
	}
	

	return;
}



