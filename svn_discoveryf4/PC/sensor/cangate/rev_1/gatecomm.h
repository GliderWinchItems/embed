/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : gatecomm.h
* Author	     : deh
* Date First Issued  : 09/19/2013
* Board              : PC
* Description        : Routines related to communications between the gateway and PC
*******************************************************************************/
#ifndef __GATECOMM_PC
#define __GATECOMM_PC

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

#include "PC_gateway_comm.h"
#include "USB_PC_gateway.h"


/* ************************************************************************************** */
int CANsendmsg(struct CANRCVBUF* pin);
/* @brief	: Send CAN msg to gateway.
 * @param	: pin = pointer to struct that is ready to go
 * @return	: 0 = OK;
 * ************************************************************************************** */

#endif

