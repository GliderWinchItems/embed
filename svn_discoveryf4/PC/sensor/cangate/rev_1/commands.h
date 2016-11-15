/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : commands.h
* Author	     : deh
* Date First Issued  : 09/19/2013
* Board              : PC
* Description        : Routines related to keyboard commands
*******************************************************************************/
#ifndef __COMMANDS_PC
#define __COMMANDS_PC


#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "common_can.h"	// Definitions common to CAN and this project.

/* ************************************************************************************** */
void do_command_keybrd(char* p);
/* @brief	: A line from the keyboard
 * @param	: p = pointer to \0 terminated string
 * ************************************************************************************** */
void do_canbus_msg(struct CANRCVBUF* p);
/* @brief	: We arrive here with a msg from the CAN BUS
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_pc_to_gateway(struct CANRCVBUF* p);
/* @brief	: FOR TESTING ####
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_printmenu(void);
/* @brief	: Print menu for the hapless Op
 * ************************************************************************************** */
void do_command_timing(void);
/* @brief	: Timeout ticks
 * @param	: p = pointer to CAN msg
 * ************************************************************************************** */
void do_command_timeout(void);
/* @brief	: Main loop encountered a timeout
 * @param	: none as yet
 * ************************************************************************************** */




#endif

