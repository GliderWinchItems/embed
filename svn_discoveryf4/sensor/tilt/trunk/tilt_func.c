/*****************************************************************************
* File Name          : tilt_func.c
* Date First Issued  : 01/22/2015
* Board              : Discovery F4
* Description        : Tilt function
*******************************************************************************/

#include "tilt_func.h"

//#include "common.h"
#include "lis302.h"
#include "bsp_uart.h"
#include "xprintf.h"

static uint32_t uxprt;
/******************************************************************************
 * void tilt_func_init(uint32_t port);
 * @brief	: Initialize
 * @param	: port = Serial port output number
*******************************************************************************/
void tilt_func_init(uint32_t port)
{
	uxprt = port;	// Save so we don't have to keep passing it as an arg.

	lis302_init();	// Init accelerometer and spi1 and countdown timer.

	return;
}

/******************************************************************************
 * void tilt_func_poll(struct CANRCVBUF *pcan);
 * @brief	: 
 * @param	: pcan = pointer to CAN msg
*******************************************************************************/
void tilt_func_poll(struct CANRCVBUF *pcan)
{
	struct LIS302AVE *plis;
	float r;
	double dx,dy,dz;

	if (pcan->id == 0x1)
	{

	}

	while ( (plis = lis302_get()) != 0)
	{
		r = 1.0 / plis->ct;
		dx = r * plis->x ;
		dy = r * plis->x ;
		dz = r * plis->x ;
		xprintf(uxprt, "%f5.1 %f5.1 %f5.1\n\r", dx, dy, dz);	
	}	
	return;


}
