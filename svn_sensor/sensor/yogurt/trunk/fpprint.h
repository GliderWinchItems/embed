/******************************************************************************
* File Name          : fpprint.h
* Date First Issued  : 08/21/2015
* Board              : f103
* Description        : Format and print floating pt
*******************************************************************************/
/*
This is used for the F103 since the launchpad compiler was not working for 
floating pt printf (which works for the 'F4).
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FPPRINT
#define __FPPRINT

#include <stdint.h>

/* **************************************************************************************/
void fmtprint(int i, float f, char* p);
/* @brief	: Format floating pt and print (%8.3f)
 * @param	: i = parameter list index
 * @param	: f = floating pt number
 * @param	: p = pointer to description to follow value
 * ************************************************************************************** */
void fpformat(char* p, double d);
/* @brief	: Format floating pt to ascii
 * @param	: p = pointer to char buffer to receive ascii result
 * @param	: d = input to be converted
 * ************************************************************************************** */

#endif 
