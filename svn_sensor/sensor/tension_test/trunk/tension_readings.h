/******************************************************************************
* File Name          : tension_readings.h
* Date First Issued  : 04/09/2015
* Board              : f103
* Description        : Initialize tension function struct with default values
*******************************************************************************/

#ifndef __TENSION_READINGS
#define __TENSION_READINGS

#include <stdint.h>
#include "common_misc.h"
#include "common_can.h"

/* **************************************************************************************/
void tension_readings_respond(struct CANRCVBUF* pcan); 
/* @brief	: Send a readings (actually send a memory location that holds a reading)
 * @param	: pcan = pointer to incoming CAN msg
 * ************************************************************************************** */


#endif 

