/******************************************************************************
* File Name          : cmd_p.h
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
#include "../../../../../svn_common/trunk/common_can.h"
//#include "../../../../../svn_common/trunk/db/can_db.h"

/******************************************************************************/
int cmd_p_init(char* p);
/* @brief 	: Get path/file from input file for program loading
 * @param	: fpList = file with path/file lines
 * @param	: 0 = init was OK; -1 = failed
*******************************************************************************/
void cmd_p_do_msg1(struct CANRCVBUF* p);
/* @brief 	: Deal with incoming CAN msgs
 * @param	: pointer to struct with CAN msg
*******************************************************************************/


extern int ldfilesopen_sw;	// 0 = .bin & .srec files have not been opened
extern int cmd_p_sw;

#endif

