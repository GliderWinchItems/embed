/******************************************************************************
* File Name          : can_driver_errors_printf.h
* Date First Issued  : 09/06/2016
* Board              : f103
* Description        : Print the error counts for the can_driver
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_DRIVER_ERRORS_PRINTF
#define __CAN_DRIVER_ERRORS_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "tension_idx_v_struct.h"
#include "can_driver.h"

/* ************************************************************************************** */
void can_driver_errors_printf(struct CAN_CTLBLOCK* pctl);
/* @brief	: Print the values since last printout
 * @param	: pctl = pointer to CAN 1 or 2 control block 
 * ************************************************************************************** */
void can_driver_errortotal_printf(struct CAN_CTLBLOCK* pctl);
/* @brief	: Print accumulated values
 * @param	: pctl = pointer to CAN 1 or 2 control block 
 * ************************************************************************************** */
#endif
