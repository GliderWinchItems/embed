/******************************************************************************
* File Name          : cable_angle_function.h
* Date First Issued  : 04/06/2015
* Board              : f103
* Description        : Cable angle function
*******************************************************************************/

#ifndef __CABLE_ANGLE_FUNCTION
#define __CABLE_ANGLE_FUNCTION

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"

struct CABLEANGLELC
{
	uint32_t	a;	// Dummy for now
};

/* **************************************************************************************/
void cable_angle_function_init(void);
/* @brief	: Initialize
 * ************************************************************************************** */

/* "Consumer" pointer to struct. */
extern struct CABLEANGLELC* pCcbl;

#endif 

