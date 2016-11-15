/******************************************************************************
* File Name          : cable_angle_function.c
* Date First Issued  : 04/06/2015
* Board              : f103
* Description        : Cable angle function
*******************************************************************************/

#include "cable_angle_function.h"
#include "can_hub.h"

struct CABLEANGLELC cable_angle_parm;

/* "Consumer" pointer to struct. */
struct CABLEANGLELC* pCcbl = &cable_angle_parm;

static struct CANHUB* phub_cable_angle;

/* **************************************************************************************
 * void cable_angle_function_init(void);
 * @brief	: Initialize
 * ************************************************************************************** */
void cable_angle_function_init(void)
{
	phub_cable_angle = can_hub_add_func();	// Set up port/connection to can_hub

	return;
}

/* **************************************************************************************
 * void cable_angle_function_poll(struct CANRCVBUF* pcan);
 * @brief	: Handle incoming CAN msgs
 * @param	; pcan = pointer to CAN msg buffer
 * @return	: Nothing for the nonce
 * ************************************************************************************** */
//void cable_angle_function_poll(struct CANRCVBUF* pcan)
//{
//	struct CANRCVBUF* x = pcan;
//	x = 0;
//	return;
//}
