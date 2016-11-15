/******************************************************************************
* File Name          : cmd_a.h
* Date First Issued  : 11/04/2014
* Board              : PC
* Description        : Same as M command but monitors ascii in CAN msgs
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
#include "../../../../../svn_common/trunk/common_can.h"
//#include "../../../../../svn_common/trunk/db/can_db.h"

/******************************************************************************/
int cmd_a_init(char* p);
/* @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
void cmd_a_do_stop(void);
/* @brief 	; Send CAN msg to disable unit sending CAN ascii msgs
*******************************************************************************/
void cmd_a_do_msg(struct CANRCVBUF* p);
/* @brief 	: Output ascii payloads for the id that was entered with the 'a' command
*******************************************************************************/


#endif

