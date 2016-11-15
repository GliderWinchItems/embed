/******************************************************************************
* File Name          : xprintf.h
* Date First Issued  : 12/04/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Substitute for 'fprintf' for multiple uarts
*******************************************************************************/

#ifndef __XPRINTF
#define __XPRINTF

/* **************************************************************************************/
int xprintf(int uartnumber, const char *format, ...);
/* @brief	: 'printf' for uartnumber 
 * @param	: uartnumber = 1-6
 * @param	: format = usual printf format
 # @param	: ... = usual printf arguments
 * @return	: Number of chars "printed"
 * ************************************************************************************** */


#endif 

