/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_controlblockptrs.c 
* Hackor             : deh
* Date First Issued  : 10/18/2010 deh
* Description        : Pointers to the possible control blocks
*******************************************************************************/ 
/*
07-14-2011 - Additions for UART4,5
*/

#include "../libusartstm32/usartall.h"

/* Control block pointers */
static struct USARTCBT* pUSARTcbt1;
static struct USARTCBT* pUSARTcbt2;
static struct USARTCBT* pUSARTcbt3;
static struct USARTCBT* pUSARTcbt4;
static struct USARTCBT* pUSARTcbt5;


static struct USARTCBR* pUSARTcbr1;
static struct USARTCBR* pUSARTcbr2;
static struct USARTCBR* pUSARTcbr3;
static struct USARTCBR* pUSARTcbr4;
static struct USARTCBR* pUSARTcbr5;


