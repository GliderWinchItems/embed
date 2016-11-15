/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_n.h
* Author	     : deh
* Date First Issued  : 09/20/2013
* Board              : PC
* Description        : Details for handling the 'n' command (list id's and msg ct/sec)
*******************************************************************************/

#ifndef __CMD_N_PC
#define __CMD_N_PC

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
void cmd_n_init(void);
/* @brief 	: Reset 
*******************************************************************************/
void cmd_n_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output current msg ID's ('n' command)
*******************************************************************************/



#endif

