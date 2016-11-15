/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : commands.c
* Author	     : deh
* Date First Issued  : 09/19/2013
* Board              : PC
* Description        : Routines related to keyboard commands
*******************************************************************************/
/*
This takes care of dispatching keyboard commands.
02-06-2014 rev 191 Add 'h' cmd
*/

#include "commands.h"
#include "timer_thread.h"
#include "cmd_a.h"
#include "cmd_f.h"
#include "cmd_h.h"
#include "cmd_l.h"
#include "cmd_m.h"
#include "cmd_n.h"
#include "cmd_p.h"
#include "cmd_q.h"
#include "cmd_r.h"

extern int fpListsw;

static u32 msg_sw;	// Command in effect
/* **************************************************************************************
 * void do_command_timeout(void);
 * @brief	: Main loop encountered a timeout
 * @param	: none as yet
 * ************************************************************************************** */
void do_command_timeout(void)
{
	switch (msg_sw)
	{
	case 'p': // Program load
//		cmd_p_timeout();
		break;

	case 261: // Command 'a' 
		break;

	default:  // Ignore all others
		break;
	}
	return;
}

/* **************************************************************************************
 * void do_command_keybrd(char* p);
 * @brief	: A line from the keyboard starts the execution of some command
 * @param	: p = pointer to \0 terminated string
 * ************************************************************************************** */
int s_cmd_onoff = 0;	// zero = don't send test msgs to CAN

void do_command_keybrd(char* p)
{

	/* This switch checks the 1st char of a keyboard entry for a command code. */
	switch(*p)
	{

	case 'a': // 'a' command (enable unit sendin ascii in CAN msgs & display)
		if (cmd_a_init(p) == 0)
		msg_sw = 'a';
		break;

	case 'd': // 'd' command
		msg_sw = 'd';
		break;

	case 'f': // Display gps fix
		msg_sw = 'f';
		break;

	case 'h': // 'h' command: histogram readout
		if (cmd_h_init(p) >= 0) // negative return means invalid input
			msg_sw = 'h';
		break;

	case 'l': // Display time from time sync msg
		msg_sw = 'l';
		break;

	case 'n': // 'n' command (list all id's and number of msgs during 1 second)
		cmd_n_init(0);	// Display counts between timer 1 sec ticks
		msg_sw = 'n';
		break;

	case 'u': // 'n' command (list all id's and number of msgs during 1 second)
		cmd_n_init(1);	// Display counts between time messages
		msg_sw = 'n';
		break;

	case 'm': // 'm' command (list msgs for the id entered)
		if (cmd_m_init(p) >= 0) // negative return means invalid input
			msg_sw = 'm';
		break;

	case 'p': // 'p' command (program loader for selected unit)
		cmd_p_init(p);	// Run edit-check on file, then load
//		msg_sw = 257 ;
		msg_sw = 'p';
		break;

	case 'q': // 'q' CAN bus loader file edit-check only.
		cmd_q_init(p);	// Run edit-check on file only.
		break;

	case 'r': // 'r' command (RESET everyone)
		cmd_r_init(p);
		break;

	case 's': // Toggle sending of test msgs to CAN bus on/off
		if (fpListsw == 0 )
		{ // Sending not enabled
			printf("A file with test msgs was not opened (use argument on command line)\n");
			break;
		}
		printf("Test msg sending to CAN bus is ");
		if (s_cmd_onoff != 0 ) 
		{
			s_cmd_onoff = 0;
			printf("OFF\n");
		}
		else
		{
			s_cmd_onoff = 1;
			printf("ON\n");
		}
		break;


	case 'x': // 'x' cancels current command
		cmd_a_do_stop();  // Disable ascii sending if enabled
		timer_thread_shutdown(); // Cancel timer thread if running
		do_printmenu();	  // Nice display for the hapless Op.
		msg_sw = 'x';
		break;

	default:
		printf("1st char not a command 0x%02x ASCII: %c\n",*p, *p);
		break;
	}
	return;
}
/* **************************************************************************************
 * void do_canbus_msg(struct CANRCVBUF* p);
 * @brief	: We arrive here with a msg from the CAN BUS
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_canbus_msg(struct CANRCVBUF* p)
{
	switch (msg_sw)
	{
	case 'a':
		cmd_a_do_msg(p);
		break;

	case 'd':
		do_pc_to_gateway(p);	// Hex listing of the CAN msg
		break;

	case 'f':
		cmd_f_do_msg(p);		// Display fix
		break;

	case 'h':
		cmd_h_do_msg(p);
		break;

	case 'l':
		cmd_l_datetime(p);
		break;

	case 'n':
		cmd_n_do_msg(p);
		break;

	case 'm':
		cmd_m_do_msg(p);
		break;

	case 'p':
		cmd_p_do_msg1(p);
		break;

	default:
		break;
	}
	return;
}

/* **************************************************************************************
 * void do_pc_to_gateway(struct CANRCVBUF* p);
 * @brief	: FOR TESTING ####
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_pc_to_gateway(struct CANRCVBUF* p)
{
	int i;
	printf("%08x %d ",p->id, p->dlc);
	for (i = 0; i < (p->dlc & 0xf); i++)
		printf("%02x ",p->cd.u8[i]);
	printf("\n");
	return;
}
/* **************************************************************************************
 * void do_printmenu(void);
 * @brief	: Print menu for the hapless Op
 * ************************************************************************************** */
void do_printmenu(void)
{
	printf("a - ascii monitor of a CAN unit\n");
	printf("d - list raw msgs\n");
	printf("f - display fix\n");
	printf("h - Photodetector level histogram from sensor\n");
	printf("l - list time/date from time CAN sync msgs\n");
	printf("n - list msg id's and msg ct during 1 sec (coarse computer timing)\n");
	printf("u - list msg id's and msg ct between CAN 1 sec time messages (precise gps timing)\n");
	printf("m - list msgs for id entered 'm xxxxxxxx (CAN ID as 8 hex digits)'\n");
	printf("p - CAN bus loader: using file for specs, edit-check and load\n");
	printf("q - CAN bus file spec edit-check only\n");
	printf("r - send high priority RESET\n");
	printf("s - Toggle sending of test msg file to CAN bus on/off\n");
	printf("x - cancel command\n");
	printf("Control C to quit program\n");
	return;
}
/* **************************************************************************************
 * void do_command_timing(void);
 * @brief	: Timeout ticks
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_command_timing(void)
{
	return;
}
