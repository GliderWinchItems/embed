/******************** (C) COPYRIGHT 2013  **************************************
* File Name          : p1_PC_handler.c
* Author             : deh
* Date First Issued  : 03/24/2013
* Board              : Sensor (USART1)
* Description        : Handler comm with PC
*******************************************************************************/
/*
05/07/2014 Sensor board mods (USART1)

A major hack of 'p1_PC_handler.c' used in the POD programs for the Olimex/CAN/CO
use.


Note: If a multi-line input is done by a cut & paste on the PC side it might not
not work correctly if the USART line buffers fill and the input line is not
removed from its buffer before being overlayed.  This would not happen with 
normal typing, and only doing one command.
*/

/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/lib/libusartstm32/usartallproto.c
@2 = svn_pod/sw_pod/trunk/pod_v1/RS232_ct.h
@3 = svn_pod/sw_pod/trunk/pod_v1/gps_packetize.c
@4 = svn_pod/sw_pod/trunk/pod_v1/tickadjust.c


*/



/*
Press the key, then return:
a - display accelerometer
b - display battery cell voltages and temp
g - monitor gps 
m - Monitor current data
s - shutdown to sleep mode now
r - Readout data backwards in time (*)
 
x - stop and return to menu
* - to interrupt and show date/time

*/

#include <stdio.h>

#include "p1_common.h"		// This has almost everything (for POD routines)
#include "gps_poll.h"
#include "p1_PC_monitor_gps.h"
#include "p1_PC_monitor_can.h"


//u16 USART1_txint_busy()	// Return 0 = buffers are free.


/* Debugging */
char cmd_save;

#define MENULINES	9
static const char *menu[MENULINES]={
"g - Display various timing parameters \n\r",\
"l - Toggle GPS sentences & date|time listing on-off\n\r",\
"m - Display of CAN msgs for a given ID and data type: mxxxxxxxx, e.g. m0000a030 for 30a0\n\r",\
"n - Send ID and data type for all CAN msgs coming\n\r",\
"s - Temporarily stop SD card writing\n\r",\
" \n\r",\
" ctl U or ctl C or ESC or Backspace to kill input\n\r",\
"x - stop and return to menu\n\r",\
"> Ready for next command\n\r"};

static const char *stoplogging = "## temporary halt to logging to SD card\n\r";

static const char *invalid_char = "Huh! 1st char not on menu, or m command not correct. hit x <enter> again\n\r";

/* USART/UART */
#include "libusartstm32/usartallproto.h"

/* Prototypes for functions in this file */
static short echo_line(void);
static short send_line(const char * p);
static int print_menu(void);
static short input_check(void);
static int abortline(struct USARTLB strlb);
static int convert_m_command (void);

/* For monitor routines to place one and only one header with each menu selection */
short usMonHeader;

/* USART related */
struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@1)

/* Holds the id for the matching of incoming CAN msgs to be sent to the PC */
u32 m_cmd_id;

/* Holds progress of 'n' command */
u32 n_cmd_state;



short 	state_PC_handler;	// State
extern	short state_readout;	// Sub-sequence start|stop times data readout (for 'state = 2' in 'p1_PC_monitor_readout.c')

/******************************************************************************
 * void p1_PC_handler_restart(void);
 * @brief 	: Send/rcv handling with PC
*******************************************************************************/
void p1_PC_handler_restart(void)
{
	state_PC_handler = 0;
	return;
}
/******************************************************************************
 * void p1_PC_handler(void);
 * @brief 	: Send/rcv handling with PC
*******************************************************************************/
void p1_PC_handler(void)
{
	short input_return;	// For debugging

	/* We enter this routine knowing that the MAX232 is running */

	input_return = input_check();	// Check for input line ready and check for valid command

	switch(state_PC_handler)
	{
	case 0:	// Initial state & "idle" state--  Send menu then wait for a command
		if (print_menu() != 0) break; 	// Keep polling until menu is sent, or at least buffered.
		state_PC_handler = 1;		// Check for input line the next time through
		monitor_gps_state = 0;
		monitor_can_state = 0;
		monitor_can_state_n = 0;
		gps_sentence_flag = 0;
		usMonHeader = 0;		// Reset the output only one header sw, if applicable


	case 1:	// Waiting for input command
		switch (input_return)		// Check for a valid input command
		{
		/* Keep coming through case 0, 1 until a valid input command received */
		case 0:	// Nothing to do. 
		case 1: // Busy responding to input.
			break;

		/* If the 'cmd' is one of the following setup to go to it next time through */

		case 'g':	// Monitor gps time setting
			state_PC_handler = 'g';
			break;
		case 'l':	// Toggle GPS sentence listing
			state_PC_handler = 'l';
			break;
		case 'm':	// Monitor current data
			state_PC_handler = 'm';
			break;
		case 'n':	// Monitor current data
			state_PC_handler = 'n';
			break;
		case 's':	// Stop logging (and therefore SD card write)
			state_PC_handler = 's';
			break;
		}
		break;

	/* Here, one of the following commands is being executed */
	/* Continue the command until cancelled by an 'x' received */
	case 'm':	// Monitor tension
		if ((input_return == 'x') || (input_return == 'm'))
		{ // Here, stop streaming
			monitor_can_state = 0;
			state_PC_handler = 0;					
		}
		else
		{ // Continue
			monitor_can_state = 1;
			p1_PC_can_monitor_msgs(); // Send CAN msgs matching ID in cmd argument
		}
		break;

	case 'n':	// Monitor tension
		if ((input_return == 'x') || (input_return == 'n'))
		{ // Here, stop streaming
			monitor_can_state_n = 0;
			state_PC_handler = 0;					
		}
		else
		{ // Continue
			monitor_can_state_n = 1;
			p1_PC_can_monitor_retrieve_id(); // Send all CAN msg IDs being received
		}
		break;

	case 's': // Stop logging command
//#include "scb.h"
//SCB_AIRCR  |= (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
			break;			

	case 'g':	// Display various timing parameters
		if ((input_return == 'x') || (input_return == 'g'))
		{ // Here, stop streaming
			state_PC_handler = 0;					
		}
		else
		{ // Continue
			p1_PC_monitor_gps();
		}
		break;


	case 'l':	// Toggle GPS sentence listing on/off
		if ((input_return == 'x') || (input_return == 'l'))
		{ // Here, stop streaming
			state_PC_handler = 0;					
		}
		else
			gps_sentence_flag = 0xff;	// See gps_packetize.c
		break;

	}
	if (input_return == 'x')
	{ // Be sure to print menu
		state_PC_handler = 0;					
	}

	return;
}
/******************************************************************************
 * short input_check(void);
 * @brief	: Check if we have received an input line from the PC
 * @return	: 0 nothing useful, 1 = polling, >1 is the cmd char
******************************************************************************/
static short state_input;
static char cmd;

static short input_check(void)
{
	switch (state_input)
	{
	case 0:	// Here, checking for an input line
		strlb = USART1_rxint_getlineboth();	// Get both char count and pointer (@1)
		/* Check if a line is ready. */
		if (strlb.p > (char*)0)			// Check if we have a completed line (@1)
		{ // Here we have a pointer to the line and a char count
			if (abortline(strlb) == 0)
			{ // Here check 1st char for a command		
				cmd = *strlb.p;			// Extract 1st char  (@1)
cmd_save = cmd;// Debugging
//printf("cmd %c strlb.ct %d\n\r",*strlb.p, strlb.ct); USART1_txint_send();

				/* Check for a valid command  */
				switch (*strlb.p)		// Check for value command  (@1)
				{
		/* If the char is not in the following list of 'case's then output the "Huh" error msg */
				case 'g':	// Monitor gps time setting
					break;
				case 'l':	// GPS sentence listing toggle
					break;
				case 'm':	// Monitor CAN data, given ID
					if (strlb.ct >= 10 )  //
					{ // 
printf("char %c strlb.ct %d\n\r", *strlb.p, strlb.ct); USART1_txint_send();
//						if (convert_m_command() == 0)
						sscanf(strlb.p+1,"%8x",(unsigned int*)&m_cmd_id);
//						{ // Here something was wrong with the input fields
//							cmd = 0;	// Consider it bogus and print error message
//						}
//						else
//						{
//printf("bbb %c strlb.ct %d\n\r",*strlb.p, strlb.ct); USART1_txint_send();
							monitor_can_state = 1;	// Set state/flag for grabbing CAN msgs
//						}
					}
					else
					{ // Line length not correct, so something is bogus about this 'd' line
						cmd = 0;
printf("char %c strlb.ct %d\n\r",*strlb.p, strlb.ct); USART1_txint_send();
					}
					break;
				case 'n':	// Send msg id's 
					break;
				case 's':	// Shut down to sleep mode now
					break;
				case 'x':	// Stop output streaming
					break;
				case '*':	// Stop and display 
					break;	
				case 0x0d:	// <enter>
					break;
				case 0x0a:
					break;

				default:	// Give up all hope, 'ye who enter here.
					cmd = 0;
					break;
				}
			}
			
			state_input = 1;	// Echo line back
		}
		return 0;

	case 1:	// Here, echo input line back
		/* Keep going through this until echo line has been sent to USART1 buffers */
		if (echo_line() != 0) return 1;	// Return: if all USART buffers full
		state_input = 2;		// Next case
		return 1;			// Return: Not finished with sequence

	case 2:	// Here, good or bad check
		if (cmd == 0)
		{ // Here invalid
printf(" %c 0x%02x ",cmd_save,cmd_save); USART1_txint_send(); // Let's look more closely at what was typed in.
			if (send_line(invalid_char) == 1) return 1; // Tell'em it's bad
			state_input = 0;	// Re-start sequence
			return 0;
		}
		else
		{ // Here, good.  Return command code
			USART1_txint_puts(" GO\n\r"); USART1_txint_send();
			state_input = 0;	// Re-start sequence
			return cmd;	
		}
	}
	return 0;
}
/******************************************************************************
 * int print_menu(void);
 * @brief	: Print menu without delaying when buffers fill
 * @return	: 0 = done/available; 1 = busy
******************************************************************************/
static short state_print;
static short line_ct;

static int print_menu(void)
{
	switch(state_print)
	{
	case 0:
		line_ct = 0;
		state_print = 1;
	case 1:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() != 0) return 1;// Return: all buffers full (@1)
		if (line_ct < MENULINES)	// Have we sent the last line?	
		{ // Here, no.  Send another
			USART1_txint_puts ((char*)menu[line_ct]);	USART1_txint_send();// (@1)
			line_ct++;	// Advance to the next line
			return 1;	// Return, not done
		}
		else
		{
			state_print = 0;	// Set back to beginning
		}
	}
	return 0;	// Show we are done.
}
/******************************************************************************
 * static short send_line(char * p);
 * @brief	: Send a line without hanging for buffer full
 * @param	: Pointer to a zero terminated string
 * @return	: 0 = done/available; 1 = busy
******************************************************************************/
static short send_line(const char * p)
{
	if (USART1_txint_busy() == 1) return 1;// Return: all buffers full (@1)

	USART1_txint_puts ((char*)p);		USART1_txint_send();	//  (@1)

	return 0;
}
/******************************************************************************
 * static short echo_line(void);
 * @brief	: Send a line without hanging for buffer full
 * @param	: Pointer to a zero terminated string
 * @return	: 0 = done/available; 1 = busy
******************************************************************************/
static short echo_line(void)
{
	if (USART1_txint_busy() == 1) return 1;// Return: all buffers full (@1)

	/* Echo line just received  (@1) */
	USART1_txint_puts ("\r\n");	// 
	USART1_txint_puts(strlb.p);	// Echo back the line just received
	USART1_txint_puts ("\n");	// Add line feed to make things look nice
	USART1_txint_send();		// Start the line buffer sending

	return 0;
}
/******************************************************************************
 * static int abortline(struct USARTLB strlb);
 * @brief	: Scan line for a char that says the op wants to abort the line
 * @param	: Pointer to struct with char count and char pointer
 * @return	: 0 = good; 1 = abort
******************************************************************************/
static int abortline(struct USARTLB strlb)
{
	int i;
	char * p = strlb.p;

	for (i = 0; i < strlb.ct; i++)
	{
		/* Check for an abort line char */
		if ( (*p == 0x18) ||	/* Control X (cancel) */ 
		     (*p == 0x1B) ||	/* Escape */ 
		     (*p == 0x08) ||	/* Backspace */ 
		     (*p == 0x15) ||	/* Control U */ 
		     (*p == 0x03) )	/* Control C */ 
			return 1;
		p++;	// Advance to next char
	}

	return 0;
}


/******************************************************************************
 * static u32 hexnib_bin (u8 p);
 * @return	: -1 = non-hex char encountered; 0 - 15 = something good
******************************************************************************/
static u32 hexnib_bin (u8 p)
{
	/* Return -1 if not a legimate HEX char. */
	if  (p <  '0') return -1;
	if  (p <= '9') return (p - '0');

	if  (p <  'A') return -1;
	if  (p <= 'F') return (p - 'A' + 10);

	if  (p <  'a') return -1;
	if  (p >  'f') return -1;
	return (p - 'a' + 10);
}
/******************************************************************************
 * static int convert_m_command (void);
 * @brief	: Convert 'm' command argument
 * @return	: 0 = BAD; non-zero = the value nicely done
******************************************************************************/
/*
Note: id recieved from the PC for the 'm' command is 8 hex chars with the byte order 
little endian.  The comparison for selecting CAN msgs requires reversing the byte order.
*/
static int convert_m_command (void)
{
	int i;
	u32 u1,u2;			// Return from hex->binary
	u8 *pm = (u8 *)&m_cmd_id;
	u8 *p  = (u8 *)(strlb.p + 1);	// Working pointer
//printf("convertA:%s\n\r",p);USART1_txint_send();
	m_cmd_id = 0;
	for (i = 0; i < 8; i++)
	{	
		if ((u1 = hexnib_bin(*p++)) == (u32)(0x0 -1) )	 return 0;	// Failure code
		if ((u2 = hexnib_bin(*p++)) == (u32)(0x0 -1) )	 return 0;	// Failure code
		*(pm + i) = ( (u1 & 0x0f) << 4 | (u2 &0x0f) );
	}
//printf("convertA:%08x\n\r",m_cmd_id);USART1_txint_send();
	return 1;		// Return with success code.
}

