/******************************************************************************
* File Name          : loopctl_print.h
* Date First Issued  : 08/17/2015
* Board              : f103
* Description        : Control loop for yogurt maker--print some things
*******************************************************************************/
/*
This is used for the F103 since the launchpad compiler was not working for 
floating pt printf (which works for the 'F4).
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOOPCTL_PRINT
#define __LOOPCTL_PRINT

#include <stdint.h>
#include "loopctl.h"

/* **************************************************************************************/
void loopctl_print(struct LOOPCTL_STATE* p);
/* @brief	: Print values in struct for monitoring 'loopctl' progress
 * @param	: p = pointer to struct with stuff to print
 * ************************************************************************************** */
void loopctl_print_hdr(void);
/* @brief	: Print column header
 * ************************************************************************************** */
void hrmn(int32_t x);
/* @brief	: Convert secs to hr:mn (toss fractional minutes)
 * @param	: x = secs ct
 * ************************************************************************************** */

#endif 
