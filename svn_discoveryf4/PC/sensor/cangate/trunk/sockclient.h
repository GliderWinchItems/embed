/******************************************************************************
* File Name          : sockclient.h
* Date First Issued  : 12-18-2013
* Board              : 
* Description        : socket client
*******************************************************************************/

#ifndef __SOCKCLIENT
#define __SOCKCLIENT

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

/* ************************************************************************************** */
int sockclient_connect(char *ip, int port);
/* @brief	: Generate a CAN test msg that goes to the CAN bus
 * @return	: Greater or equal zero is success--
 *		:    file descriptor
 *		: Negative--error
 * ************************************************************************************** */

#endif 

