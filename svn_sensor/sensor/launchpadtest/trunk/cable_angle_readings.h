/******************************************************************************
* File Name          : cable_angle_readings.h
* Date First Issued  : 04/13/2015
* Board              : f103
* Description        : Initialize cable_angle function struct with default values
*******************************************************************************/

#ifndef __CABLE_ANGLE_READINGS
#define __CABLE_ANGLE_READINGS

#include <stdint.h>
#include "common_misc.h"
#include "common_can.h"

/* **************************************************************************************/
void cable_angle_readings_respond(struct CANRCVBUF* pcan); 
/* @brief	: Send a readings (actually send a memory location that holds a reading)
 * @param	: pcan = pointer to incoming CAN msg
 * ************************************************************************************** */


#endif 

