/******************************************************************************
* File Name          : cmd_q.h
* Date First Issued  : 12/27/2014
* Board              : PC
* Description        : CAN bus loader file: edit-check only
*******************************************************************************/

#ifndef __CMD_Q_PC
#define __CMD_Q_PC

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
#include "common_highflash.h"
#include "parse.h"

/******************************************************************************/
int cmd_q_init(char* p);
/* @brief 	: edit-check CAN bus loader file
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/
void cmd_q_subsystems(struct LDPROGSPEC *p, int idsize);
/* @brief 	: List subsystems
 * @param	: p = pointer to struct array holding input file data
 * @param	: idsize = size of array
*******************************************************************************/

#endif

