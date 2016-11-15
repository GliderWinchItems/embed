/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_r.h
* Author	     : deh
* Date First Issued  : 09/22/2013
* Board              : PC
* Description        : Send master reset
*******************************************************************************/

#ifndef __CMD_R_PC
#define __CMD_R_PC

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
int cmd_r_init(char* p);
/* @brief 	: Send RESET CAN msg to gateway
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/

#endif

