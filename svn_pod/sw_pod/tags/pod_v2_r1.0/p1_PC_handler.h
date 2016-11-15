/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_handler.h
* Author             : deh
* Date First Issued  : 09/05/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Handler comm with PC
*******************************************************************************/

#ifndef __PC_HANDLER
#define __PC_HANDLER

/*
This goes in the mainline polling loop.

*/

/* Readback start/stop input time field.  Two unsigned long in hex */
// 'd11122921451112290945' [This is what we expect.]
#define SIZESTARTSTOP	26	// Minimum umber of chars in a valid input line


/******************************************************************************/
void p1_PC_handler(void);
/* @brief 	: Send/rcv handling with PC
*******************************************************************************/
void p1_PC_handler_restart(void);
/* @brief 	: Send/rcv handling with PC
*******************************************************************************/

/* For monitor routines to place a header */
extern short usMonHeader;

/* Toggles load_cell zeroing on/off when the 'm' command is in effect */
extern unsigned int uiLoad_cell_zeroing;	// 0 = OFF, not-zero = ON.
extern unsigned int uiLoad_cell_zero_save;	// 0 = don't save; not-zero = save in calibration table


/* Flag that wills top adjusting time to gps (so we can see how well the
32 KHz osc compensation is working) (@3) (@4) */
extern unsigned char gps_timeadjustflag;	// 0 = adjust time; 1 = stop adjusting time

/* Start/stop date times (in tick counts) for selecting readback data to send to PC */
extern unsigned long ulreadbackstop;
extern unsigned long ulreadbackstart;

/* Input calibration--last good value received */
extern struct TWO two_f;	// n1 = table position: 0-63; n2 = value

extern unsigned int Debugpktctr;

#endif

