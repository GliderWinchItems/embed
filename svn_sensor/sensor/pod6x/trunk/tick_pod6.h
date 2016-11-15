/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : tick_pod.h
* Hackerees          : deh
* Date First Issued  : 02/16/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Post SYSTICK interrupt handling
*******************************************************************************/


#ifndef __TICK_POD6
#define __TICK_POD6

/******************************************************************************/
int tick_pod6_init(void);
/* @brief 	: Enable preparation and CAN xmt of measurement
 * @return	: 0 = success; -1 = CAN control block pointer pctl1 was NULL
*******************************************************************************/

/* Pointer to next routine if these are chained */
// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_ER (very low priority) -> Here -> Next (or return)
extern void 	(*Continuation_of_I2C2_ER_IRQHandler_ptr)(void);

/* Error counter */
extern u32	can_msgovrflow_6;		// Count times xmt buffer was full

#endif 

