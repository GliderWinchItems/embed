/******************************************************************************
* File Name          : xprintf.c
* Date First Issued  : 12/04/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Substitute for 'fprintf' for multiple uarts
*******************************************************************************/


/* **************************************************************************************
 *  int xprintf(int uartnumber, const char *format, ...);
 * @brief	: 'printf' for uartnumber 
 * @param	: uartnumber = 1-6
 * @param	: format = usual printf format
 # @param	: ... = usual printf arguments
 * @return	: Number of chars "printed"
 * ************************************************************************************** */
#include <stdarg.h>
#include <stdio.h>
#include "bsp_uart.h"

int xprintf(int uartnumber, const char *fmt, ...)
{
	va_list argp;
	char vv[256];\
	int r;
	va_start(argp, fmt);
	va_start(argp, fmt);
	r = vsprintf(vv, fmt, argp);
	va_end(argp);
	bsp_uart_puts_uartnum(uartnumber,vv);
	return r;
}

