/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_p.h
* Author	     : deh
* Date First Issued  : 09/21/2013
* Board              : PC
* Description        : Program load for one unit
*******************************************************************************/

#ifndef __CMD_P_PC
#define __CMD_P_PC

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

/******************************************************************************/
int cmd_p_init(char* p);
int cmd_p_init1(char* p);
int cmd_p_init2(char* p);
/* @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
 ******************************************************************************/
void cmd_p_timeout(void);
/* @brief 	: 
*******************************************************************************/
void cmd_p_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/

extern int ldfilesopen_sw;	// 0 = .bin & .srec files have not been opened

#endif

