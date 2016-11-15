/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_l.h
* Author	     : deh
* Date First Issued  : 09/20/2013
* Board              : PC
* Description        : List time from time sync msgs on CAN BUS
*******************************************************************************/

#ifndef __CMD_L_PC
#define __CMD_L_PC

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
void cmd_l_datetime(struct CANRCVBUF* p);
/* @brief	: Format and print date time from time sync msg in readable form
 ******************************************************************************/


#endif

