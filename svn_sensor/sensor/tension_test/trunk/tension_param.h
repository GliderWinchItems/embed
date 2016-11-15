/******************************************************************************
* File Name          : tension_param.h
* Date First Issued  : 04/08/2015
* Board              : f103
* Description        : Initialize tension function struct with default values
*******************************************************************************/
#ifndef __TENSION_PARAM_Q
#define __TENSION_PARAM_Q

#include <stdint.h>
#include "common_misc.h"
#include "common_can.h"
#include "tension_function.h"

/* **************************************************************************************/
void tension_param_default_init(struct TENSIONLC* pten); 
/* @brief	: Initialize struct with default values
 * ************************************************************************************** */

#endif 

