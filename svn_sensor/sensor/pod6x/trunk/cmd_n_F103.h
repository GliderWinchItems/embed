/******************************************************************************
* File Name          : cmd_n_F103.h
* Date First Issued  : 05-28-2015
* Board              : F103 or F4
* Description        : CAN routines common for F103 and F4 CAN drivers
*******************************************************************************/
/* 
*/

#ifndef __STM32_CMD_N_F103
#define __STM32_CMD_N_F103

/* **************************************************************************************/
int32_t cmd_n_init_F103(unsigned int count);
/* @brief	: Purloin some memory for this mess
 * @param	: count = size of CAN id table
 * @return	: 0 = OK, -1 = fail
 * ************************************************************************************** */
void cmd_n_F103(struct CANRCVBUF* p);
/* @brief	: make list of different CAN ids and counts
 * @param	: p = Pointer to CAN msg
 * ************************************************************************************** */



#endif
