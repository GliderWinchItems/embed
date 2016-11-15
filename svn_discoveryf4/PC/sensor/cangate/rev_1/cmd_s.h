/******************************************************************************
* File Name          : cmd_s.h
* Date First Issued  : 12/10/2013
* Board              : PC
* Description        : Send test msgs to CAN gateway
*******************************************************************************/

#ifndef __CMD_S_PC
#define __CMD_S_PC

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
void cmd_s_do_msg(FILE* fpList, int fd);
/* @brief 	: Send CAN msgs
 * @param	: fpList = file with test msgs
 * @param	: fd = file descriptor for serial port
*******************************************************************************/

#endif

