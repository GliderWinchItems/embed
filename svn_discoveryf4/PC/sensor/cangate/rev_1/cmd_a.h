/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_a.h
* Author	     : deh
* Date First Issued  : 09/26/2013
* Board              : PC
* Description        : Loader sequence for one unit
*******************************************************************************/

#ifndef __CMD_A_PC
#define __CMD_A_PC

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
int cmd_a_init(char* p);
/* @brief 	: 
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/
void cmd_a_do_msg(struct CANRCVBUF* p);
/* @brief 	: 
*******************************************************************************/
void cmd_a_timeout(void);
/* @brief 	: 
*******************************************************************************/

#endif

