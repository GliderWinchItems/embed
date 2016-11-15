/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_handler.c
* Author             : deh
* Date First Issued  : 09/05/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Handler comm with PC
*******************************************************************************/
/*


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

#include "p1_common.h"		// This has almost everything 
#include "RS232_ctl.h"

//u16 USART1_txint_busy()	// Return 0 = buffers are free.


/* Debugging */
char cmd_save;

#define MENULINES	19
static const char *menu[MENULINES]={
"a - display accelerometer\n\r",\
"b - display battery cell voltages and temp\n\r",\
"c - stop gps time adjusting (calibration)\n\r",\
"d - set date/time for start:stop data readout\n\r",\
"f - calibration table: input new value\n\r",\
"g - monitor gps \n\r",\
"m - Monitor current data\n\r",\
"s - shutdown to sleep mode now\n\r",\
"r - Readout data backwards in time (*)\n\r",\
"e - Reset readout most recent\n\r",\
" \n\r",\
" ctl U or ctl C or ESC or Backspace to kill input\n\r",\
"x - stop and return to menu\n\r",\
"* - (when running r or d): to interrupt and show date/time\n\r",\
"z - (when running m): toggle load-cell zeroing on/off during m command\n\r",\
"v - (when running f): toggle old/new calibrations\n\r",\
"p - Save memory calibrations to SD when command s (sleep) executed\n\r",\
"w - Do not save memory calibrations to SD when command s (sleep) executed\n\r",\
"> Ready for next command\n\r"};

static const char *shutline = "## deepsleep\n\r";
static const char *resetline ="#### Reset 'r' command to latest packet completed\n\r\n\r";

static const char *invalid_char = "Huh! 1st char not on menu, or d command error\n\r";


/* USART/UART */
#include "libusartstm32/usartallproto.h"

/* Prototypes for functions in this file */
static short echo_line(void);
static short send_line(const char * p);
static int print_menu(void);
static short input_check(void);
static int abortline(struct USARTLB strlb);
static int convert_d_command (void);
static int convert_f_command (char * p);



/* Flag that wills top adjusting time to gps (so we can see how well the
32 KHz osc compensation is working) (@3) (@4) */
unsigned char gps_timeadjustflag;	// 0 = adjust time; 1 = stop adjusting time

/* For monitor routines to place one and only one header with each menu selection */
short usMonHeader;

/* USART related */
struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@1)

/* Toggles load_cell zeroing on/off when the 'm' command is in effect */
unsigned int uiLoad_cell_zeroing;	// 0 = OFF, not-zero = ON.
unsigned int uiLoad_cell_zero_save;	// 0 = don't save; not-zero = save in calibration table

/* Start/stop date times (in tick counts) for selecting readback data to send to PC */
unsigned long ulreadbackstop;
unsigned long ulreadbackstart;


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

	input_return = input_check();

	switch(state_PC_handler)
	{
	case 0:	// Initial state & "idle" state--  Send menu then wait for a command
		if (print_menu() != 0) break; 	// Keep polling until menu is sent, or at least buffered.
		state_PC_handler = 1;		// Check for input line the next time through
		usMonHeader = 0;		// Reset the output only one header sw, if applicable
		ulreadbackstop = 0;		// Reset stop date/time for readback
		ulreadbackstart = 0;		// Reset start date/time for readback
		p1_PC_monitor_inputcal(0);	// Reset calibration input sequence


	case 1:	// Waiting for input command
		switch (input_return)		// Check for a valid input command
		{
		/* Keep coming through case 0, 1 until a valid input command received */
		case 0:	// Nothing to do. 
		case 1: // Busy responding to input.
			break;

		/* If the 'cmd' is one of the following setup to go to it next time through */

		case 'm':	// Monitor current data
			state_PC_handler = 'm';
			break;
		case 'r':	// Readout data (backwards in time (*))
			state_PC_handler = 'r';
			break;
		case 'e':	// Reset readout pointer
			state_PC_handler = 'e';
			break;
		case 's':	// Shut down to sleep mode now
			// Set some sort of flag for 'main'
			state_PC_handler = 's';
			break;
		case 'g':	// Monitor gps
			state_PC_handler = 'g';
			break;
		case 'c':	// Calibration with gps
			state_PC_handler = 'c';
			break;
		case 'b':	// Display battery voltages and temp
			state_PC_handler = 'b';
			break;
		case 'a':	// Display accelerometer
			state_PC_handler = 'a';
			break;
		case 'd':	// Readout data backwards in time, between two date/times
			state_PC_handler ='r';
			break;
		case 'f':	// Input a new calibration table value
			state_PC_handler ='f';
			break;

		}
		break;

	/* Here, one of the following commands is being executed */
	/* Continue the command until cancelled by an 'x' received */
	case 'm':	// Monitor tension
		if (input_return == 'x')
		{ // Here, stop streaming
			state_PC_handler = 0;		
		}
		else
		{ // Continue 
			/* Toggle load cell zeroing on/off */
			if (input_return == 'z') uiLoad_cell_zeroing ^= ~0;

			/* Save zeroing in calibration table */
			if (input_return == 'p') 
			{
				uiLoad_cell_zero_save = 1;
			}

			RS232_ctl_reset_timer();	// Reset MAX232 timeout counter @2
			p1_PC_monitor();
		}
		break;

	case 'r':	// The 'd' command also comes here, but with the start|stop times set.
/* email 10/04/2011 02:53:58 PM: 2. Doing an "sdlog_flush()" before you start to read is a good idea. */
		if (input_return == 'x')	// Abort/stop readout
		{ // Here, stop streaming
			state_PC_handler = 0;	// Terminate
			state_readout = 0; 	// Reset subsequence for start|stop times ('p1_PC_monitor_readout.c' 'case 2')
		}
		else
		{ // Continue
			if (input_return == '*')
			{
				p1_PC_monitor_readout_datetime();	
				state_PC_handler  = 4;	// Idle until <enter> is hit
				break;
			}
			RS232_ctl_reset_timer();	// Reset MAX232 timeout counter @2
			if ( (p1_PC_monitor_readout()) == 1 )	// End of SD?
			{	
				sdlog_seek(~0ULL);	// Reset readout back to start with most recent
				state_PC_handler = 0;		// Terminate 
			}
//p1_PC_monitor_readout();
		}
		break;

	case 4:	// Pause after showing date/time
		if ((input_return == 0x0d)||(input_return == 0x0a)) // <enter> resumes readback
			state_PC_handler  = 'r';
		break;

	case 'e':
		if ( (send_line((const char*)resetline)) == 0)
		{
			sdlog_seek(~0ULL);	// Reset readout back to start with most recent
			state_PC_handler = 0;				
		}
		break;
		

	case 's': // Shutdown command
			shutdownflag = 1;	// Set shutdown flag
			if ( (send_line((const char*)shutline)) == 0)
			{
				p1_shutdown_normal_run();
				state_PC_handler = 0;				
			}
			else
			{
				state_PC_handler = 3;
			}
			break;			
	case 3:
			if (send_line((const char*)shutline) == 0)
			{
				p1_shutdown_normal_run();
				state_PC_handler = 0;			 	
			}
			break;

	case 'g':	// Monitor GPS v rtc tick counter
		if (input_return == 'x')
		{ // Here, stop streaming
			gps_monitor_flag = 0;		// Let MAX232 timeout
			state_PC_handler = 0;					
		}
		else
		{ // Continue
			RS232_ctl_reset_timer();	// Reset MAX232 timeout counter @2
			gps_monitor_flag = 1;		// Keep gps alive
			p1_PC_monitor_gps();
		}
		break;

	case 'c':	// GPS calibration
		if (input_return == 'x')
		{ // Here, stop streaming
			gps_monitor_flag = 0;		// Let MAX232 timeout
			gps_timeadjustflag = 0;		// Let GPS adjust time
			state_PC_handler = 0;					
		}
		else
		{ 
			RS232_ctl_reset_timer();	// Reset MAX232 timeout counter @2
			gps_monitor_flag = 1;		// Keep gps alive
			gps_timeadjustflag = 1;		// Stop GPS adjusting time
			p1_PC_monitor_gps();
		}
		break;

	case 'b':	// Monitor battery & thermistor
		if (input_return == 'x')
		{ // Here, stop streaming
			state_PC_handler = 0;					
		}
		else
		{ // Continue
			RS232_ctl_reset_timer();	// Reset MAX232 timeout counter @2
			p1_PC_monitor_batt();
		}
		break;
	case 'a':	// Monitor accelerometer
		if (input_return == 'x')
		{ // Here, stop streaming
			state_PC_handler = 0;					
		}
		else
		{ // Continue
			RS232_ctl_reset_timer();	// Reset MAX232 timeout counter @2
			p1_PC_monitor_accel();
		}
		break;
	case 'f':	// Input new calibration table value
		if (input_return == 'x')
		{ // Here, stop command
			state_PC_handler = 0;					
			p1_PC_monitor_inputcal(0);
		}
		else
		{ // Continue
			RS232_ctl_reset_timer();	// Reset MAX232 timeout counter @2
			p1_PC_monitor_inputcal(1);
		}
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
				RS232_ctl_reset_timer();	// Reset MAX232 timeout counter (@2)
				cmd = *strlb.p;			// Extract 1st char  (@1)
cmd_save = cmd;// Debugging
				/* Check for a valid command (first char, except for 'd' command) */
				switch (*strlb.p)		// Check for value command  (@1)
				{
		/* If the char is not in the following list of 'case's then output the "Huh" error msg */
				case 'a':	// Display accelerometer
					break;
				case 'b':	// Display battery voltages and temp
					break;
				case 'c':	// gps calibration
					break;
				case 'e':	// Reset readout pointer
					break;
				case 'g':	// Monitor gps
					break;
				case 'm':	// Monitor current data
					break;
				case 'p':	// Write calibrations to SD card
					cCalChangeFlag = 1;	// We changed the calibration so set flag to cause update when we shutdown ('s' command)
					break;
				case 'w':	// Do not write calibrations to SD card
					cCalChangeFlag = 0;	// We changed the calibration so set flag to cause update when we shutdown ('s' command)
					break;
				case 'v':	// Reverse old and new calibration value
					p1_PC_monitor_inputcal(2);
					break;
				case 'r':	// Readout data (backwards in time (*)
					break;
				case 's':	// Shut down to sleep mode now
					break;
				case 'x':	// Stop output streaming
					break;
				case 'z':	// Toggle load-cell averaging
					break;
				case '*':	// Stop and display 
					break;	
				case 0x0d:	// <enter>
					break;
				case 0x0a:
					break;
				case 'f':
					if (convert_f_command(strlb.p) != 0)
					{ // Here something wrong with the fields
						cmd = 0;
					}
					break;
				case 'd':	// Input line *should* have date/time for start stop
//printf("d: %u\n\r",strlb.ct);USART1_txint_send();
					if (strlb.ct >= SIZESTARTSTOP)  // (p1_PC_monitor_readout.h)
					{ // Check for valid start|stop time and convert to tick counts
						if (convert_d_command() != 0)
						{ // Here something was wrong with the input fields
							cmd = 0;	// Consider it bogus and print error message
						}
						else
						{ // Here, we have setup the start|stop times.  So treat it like a 'r' command
							cmd = 'r';
						}
					}
					else
					{ // Line length not correct, so something is bogus about this 'd' line
						cmd = 0;
					}
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
		RS232_ctl_reset_timer();	// Reset MAX232 timeout counter (@2)
		/* Keep going through this until echo line has been sent to USART1 buffers */
		if (echo_line() != 0) return 1;	// Return: all USART buffers full
		state_input = 2;	// Next case
		return 1;	// Return: Not finished with sequence

	case 2:	// Here, good or bad check
		RS232_ctl_reset_timer();	// Reset MAX232 timeout counter (@2)
		if (cmd == 0)
		{ // Here invalid

printf(" 0x%02x ",cmd_save); USART1_txint_send(); // Let's look more closely at it.

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
 * static int convert_d_command (void);
 * @brief	: Convert 'd' command start/stop times to unsigned longs
 * @return	: 0 = OK; non-zero = not usable.
******************************************************************************/
static time_t PCtolinuxtime(char * p);
static int convert_d_command (void)
{
	unsigned long ul1,ul2;
	int i;
	
	/* Convert first field */
	ul1 = PCtolinuxtime (strlb.p+1);
	if (ul1 == 0) return 1;	// We should not have a zero date/time

	/* Convert second field */
	/* Check special case of all zero */
	for (i = 0; i < 12; i++)
	{
		if ( *(strlb.p+13+i) != '0') break;
	}

	if (i == 12) 
	{
		ul2 = 0;
	}
	else
	{
		ul2 = PCtolinuxtime (strlb.p+13);
		if (ul2 == 0) return 2;
	}

	/* Make sure we have them in the sequence needed later */
	if (ul1 > ul2)
	{
		ulreadbackstart = ul1;
		ulreadbackstop  = ul2;
	}
	else
	{
		ulreadbackstop  = ul1;
		ulreadbackstart = ul2;
	}
printf("start: %9u %s",ulreadbackstart,ctime((const time_t*)&ulreadbackstart));
USART1_txint_send();// (@1)
printf("stop : %9u %s",ulreadbackstop,ctime((const time_t*)&ulreadbackstop));
USART1_txint_send();// (@1)

	/* Make date/time into epoch shifted form, but don't shift left to make ticks */
	if (ulreadbackstop != 0)
		ulreadbackstop   -= PODTIMEEPOCH;
	ulreadbackstart  -= PODTIMEEPOCH;

printf("start: %9u\n\rstop : %9u\n\r",ulreadbackstart,ulreadbackstop);
USART1_txint_send();// (@1)

	return 0;	// Return with sucess code.
}
/******************************************************************************
 * time_t PCtolinuxtime(char * p);
 * @brief	: Convert yymmddhhmm to linux time
 * @param	: p - pointer to 'd' command line
 * @return	: 0 = record is not valid; not zero = linux time
 ******************************************************************************/
static int ascii_bin(char * p);
static time_t PCtolinuxtime(char * p)
{
/* Example of d command line to convert
yymmddhhmmyymmddhhmm for start|stop date/times
d11122921451112290945
*/

	struct tm t;

	/* Fill tm struct with values extracted from gps */
	t.tm_sec =	 ascii_bin(p+10);
	t.tm_min = 	 ascii_bin(p+8);
	t.tm_hour =	 ascii_bin(p+6);
	t.tm_mday =	 ascii_bin(p+4);
	t.tm_mon =	 ascii_bin(p+2) - 1;
	t.tm_year =	 ascii_bin(p+0) + (2000 - 1900);

//printf("tm: %u\n\r",t.tm_mon); USART1_txint_send();	

	return mktime (&t);	// Convert to time_t format
}
/******************************************************************************
 * static void ascii_bin(char * p);
 * @brief	: Convert two ascii to binary
 * @param	: pointer to high order ascii
 * @return	: binary number 0 - 99
 ******************************************************************************/
static int ascii_bin(char * p)
{
	int temp;
	temp = *p++ - '0';
	return (10 * temp) + *p - '0';
}
/******************************************************************************
 * static struct TWO atoifield (char *p);
 * @brief	: A field to decimal and check for errors (' -12345 ')
 * @return	: TWO.n1: Edit 0 = NG, not zero = pointer; TWO.n2 = signed number converted
******************************************************************************/
static struct TWO atoifield (char *p)
{
#define FIELDLIMIT	25	// Limit search forward 

	struct TWO tw = {0,0};
	int i = 0;	// Limit in search forward
	int sw = 0;	// 0 = positive number; -1 = negative number

	/* There must be a leading blank */
	if (*p != ' ') return tw;

	/* Spin forward to first non-blank */
	while ((*p == ' ') && (i++ < FIELDLIMIT)) p++;;
	if (i >= FIELDLIMIT) return tw;

	/* Handle negative numbers */
	if (*p == '-') 
	{
		sw = -1;	// After conversion to decimal make number negative
		p ++;		// Advance to char following '-'
	}

	/* Convert chars to decimal */
	i = 0;	// Prevent this from running amok
	tw.n2 = 0;	// Decimal return number to be built
	
	/* Take chars and convert to decimal until the end of the field is detected */
	while  ( (!( (*p == ' ') || (*p == 0x0a) || (*p == 0x0d) || (*p == 0) ) ) && (i++ < FIELDLIMIT) )
	{
		if  ((*p <= '9') && (*p >= '0'))
		{
			tw.n2 = tw.n2 * 10 + (*p - '0');
			p++;
		}
		else
		{
			return tw;	// Return with tw.n1 showing an error code.
		}
	}
	if (i >= FIELDLIMIT) return tw;	// Check if the while terminated because of too many chars

	/* Make negative if necessary */
	if (sw != 0) tw.n2 = -tw.n2;

	/* Show that result is OK by returning the pointer to next char */
	tw.n1 = (unsigned int)p;
	
	return tw;
}

/******************************************************************************
 * static struct TWO convert_field (char * p);
 * @brief	: Convert 'd' command start/stop times to unsigned longs
 * @return	: 0 = OK; non-zero = not usable.
******************************************************************************/
struct TWO two_f;

static int convert_field (char *p)
{
	struct TWO tw = {0,0};

/* 
We expect 'f xx n...n' where the first field, xx, is 1 - 63, and
the second field, n...n, can be a +/- (i.e. a signed int) of any
size that fits in an 'int'.

The first field is the index into the table of calibration values.  The first
entry (index = 0) is the unit/version number so changing this is not allowed.
Hence, the permitted range for this field/index is 1 - 63.
*/

//*(strlb.p + 6) = 0;
printf(" Z:%s \n\r",p); USART1_txint_send();
	/* Convert 1st field to decimal */
	tw = atoifield(p+1);
	if (tw.n1 == 0) return 1;	// Edit error in conversion
	if ((tw.n2 >= 64) || (tw.n2 < 1)) return 2; // Position field out of range

	two_f.n1 = tw.n2;	// Save 1st field (position)
	
	/* Convert second field */
	tw = atoifield((char *)tw.n1);	// Convert 2nd field
	if (tw.n1 == 0) return 3;	// Edit error in conversion
	// There is no range check on the calibration value
	
	two_f.n2 = tw.n2;	// Save second field

	return 0;
}

/******************************************************************************
 * static int convert_f_command (char *p);
 * @brief	: Convert 'd' command start/stop times to unsigned longs
 * @return	: 0 = OK; non-zero = not usable.
******************************************************************************/
static int convert_f_command (char * p)
{
	int x = convert_field(p);	// Convert the two fields
	
	switch (x)
	{
	case 0:	// Here, input field passed edits in conversion
		printf(" Position: %d, Value: %d\n\r", two_f.n1, two_f.n2); USART1_txint_send();
		return 0;
		break;	
	case 1: // Here, the first field failed
		USART1_txint_puts(" First field failed edit\n\r"); USART1_txint_send();
		return 1;
		break;
	case 2: // Here, the second field failed
		USART1_txint_puts(" Second field failed edit\n\r"); USART1_txint_send();
		return 1;
		break;
	default:
		break;
	}
	return 1;
}


