/******************************************************************************
* File Name          : can_nxp_setRS.h
* Date First Issued  : 06-25-2015
* Board              : F103: POD or sensor boards
* Description        : Set the RS pin on the CAN driver for high-speed or standby
*******************************************************************************/

#ifndef __CAN_NXP_SET
#define __CAN_NXP_SET

/*******************************************************************************/
void can_nxp_setRS(int rs, int board);
/* @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs) (SYSTICK version)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @param	: board: 0 = POD, 1 = sensor RxT6 board
 * @return	: Nothing for now.
*******************************************************************************/



#endif
