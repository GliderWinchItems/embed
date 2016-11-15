/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_deepsleep_run.h
* Author             : deh
* Date First Issued  : 09/01/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Deepsleep: do battery check and time/temp adjust
*******************************************************************************/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/stm32/trunk/devices/Podpinconfig.h

@1 = svn_pod/stm32/trunk/devices/32KHz.c


*/
	
#include "p1_common.h"
#include "p1_normal_run.h"

/******************************************************************************
 * void p1_deepsleep_run(void);
 * @brief 	: Do necessary housekeeping and return to deepsleep
*******************************************************************************/
void p1_deepsleep_run(void)
{

//p1_normal_run();	// Setup and do tension logging, etc.

	/* Finish initialization needed to deepsleep mode */
	p1_initialization_deepsleep();

	/* Loop until tickadjust is complete, then set shutdown flag
           and loop until the shutdown routine completes the process. */
	while (1==1)
	{
		if (TickAdjustOTO == 1)
			shutdownflag = 1;
		/* Check for shutdown: no activity timeout, battery low, PC shutdown command, whatever */
		p1_shutdown_normal_run();	// Upon timout we never return from this, i.e. go into STANDBY deepsleep		
	}


	/* Get battery and go to sleep forever if low */

	/* do time/temp asjust */
//	gotostandby();	// 10 sec STANDBY


	/* Never comes back to here */

}

