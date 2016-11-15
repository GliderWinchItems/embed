/******************************************************************************
* File Name          : f4dreset.h
* Date First Issued  : 08/15/2014
* Board              : Discovery F4
* Description        : Auto reset for non-cold startup
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __F4DAUTORESET
#define __F4DAUTORESET

/******************************************************************************/
void f4dreset(void);
/* @brief 	: Reset if we detect that start was no a cold start
*******************************************************************************/
unsigned int f4dreset_getreg(unsigned int x);
/* @brief 	: Unlock and return a backup a register
 * @brief	: After the 1st unlock r/w to the registers can be done directly.
 * @param	: x = Register number 0 - 19
 * @return	: backup register
*******************************************************************************/



#endif

