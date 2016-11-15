/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : commands.c
* Author	     : deh
* Date First Issued  : 09/19/2013
* Board              : PC
* Description        : Routines related to keyboard commands
*******************************************************************************/
/*
This takes care of dispatching keyboard commands.

*/

#include "commands.h"
#include "cmd_a.h"
#include "cmd_l.h"
#include "cmd_m.h"
#include "cmd_n.h"
#include "cmd_p.h"
#include "cmd_r.h"


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
		cmd_p_timeout();
		break;

	case 261: // Command 'a' 
		cmd_a_timeout();
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

void do_command_keybrd(char* p)
{
	/* The following switch handles keyboard entries following the start of a command. */
	/* The switch codes have values greater than one ascii char */
	switch(msg_sw)
	{ // Here, these are "follow-on" inputs initated by the single char command code.

	/* Followup keyboard entries for 'p' command */
	case 257: // 'p' cmd: Hapless Op has entered path/name or selection number
		if (cmd_p_init1(p) == 2) msg_sw = 258; else msg_sw = 0;
		return;
	case 258: // 'p' cmd: Hapless Op has entered a path/name
		if (cmd_p_init2(p) == 2) cmd_p_init(p); else msg_sw = 0;
		return;

	/* Followup keyboard entries for 'a' command */
	case 259: // 'a' cmd: Hapless Op has entered path/name or selection number
		if (cmd_p_init1(p) == 2) msg_sw = 260; else msg_sw = 261;
		return;
	case 260: // 'a' cmd: Hapless Op has entered a path/name
		if (cmd_p_init2(p) == 0) msg_sw = 261; else msg_sw = 0;
		return;
	case 261: // 'a' cmd: We have the loading files .bin and .srec opened
		return;
	}

	/* This switch checks the 1st char of a keyboard entry for a command code. */
	switch(*p)
	{

	case 'a': // 'a' command (load a unit)
		if (cmd_a_init(p) == 0)
			msg_sw = 'a';
		break;

	case 'd': // 'd' command
		msg_sw = 'd';
		break;

	case 'l': // Display time from time sync msg
		msg_sw = 'l';
		break;

	case 'n': // 'n' command (list all id's and number of msgs during 1 second)
		cmd_n_init();
		msg_sw = 'n';
		break;

	case 'm': // 'm' command (list msgs for the id entered)
		if (cmd_m_init(p) >= 0) // negative return means invalid input
			msg_sw = 'm';
		break;

	case 'p': // 'p' command (program loader for selected unit)
		cmd_p_init(p);	// List the list file with path/names
		msg_sw = 257 ;
		break;

	case 'r': // 'r' command (RESET everyone)
		cmd_r_init(p);
		break;


	case 'x': // 'x' cancels current command
		do_printmenu();
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
	case 261: // Command 'a': .bin and .srec files are open and ready for loading
		cmd_a_do_msg(p);

	case 'd':
		do_pc_to_gateway(p);	// Hex listing of the CAN msg
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
		cmd_p_do_msg(p);
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
	printf("a - load program \n");
	printf("d - list raw msgs\n");
	printf("l - list time/date from time CAN sync msgs\n");
	printf("n - list msg id's and msg ct during 1 sec\n");
	printf("m - list msgs for id entered 'm xxxx'\n");
	printf("p - select unit to program load from list, or enter path/name, and open .bin & .srec files\n");
	printf("r - send high priority RESET\n");
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
