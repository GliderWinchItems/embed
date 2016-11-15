/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_cold_run.c
* Hackeroos          : caw, deh
* Date First Issued  : 08/30/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Sort out type of reset and some other related things ;)
*******************************************************************************/
/*
08-30-2011
*/

/*


*/
#include "p1_common.h"

/******************************************************************************
 * void p1_cold_run(void);
 * @brief 	: Do the powered down (cold) start up sequence
*******************************************************************************/
 void p1_cold_run(void)
{

	p1_normal_run();	// At this point continue as if it was powered before reset

	return;
}
