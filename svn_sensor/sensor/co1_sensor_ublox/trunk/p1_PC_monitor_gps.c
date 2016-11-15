/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : p1_PC_monitor_gps.c
* Author             : deh
* Date First Issued  : 03/25/2013
* Board              : Sensor board (USART1)
* Description        : Output current gps to PC with command 'g'
*******************************************************************************/
/*
05/07/2014 - mods for sensor board (USART1)

A mess pulled from earlier versions of 'co1.c' that were used for debugging the
the time phasing.

*/




#include "p1_gps_time_convert.h"
#include "p1_common.h"
#include "p1_PC_handler.h"
#include "calendar_arith.h"
#include "common_can.h"
#include "common_time.h"
#include <time.h>

/* ============ DEBUGGING ====================== */
static unsigned int errctr;
static int cycntHI = -32000000;
static int cycntLO = +32000000;

extern unsigned int ticks;
extern volatile int ticks_dev;
extern volatile unsigned int ticks_flg;
static unsigned int ticks_flg_prev = 0;

extern volatile u32 canlogSDwritectr;

extern volatile u32	tim4_tickspersec_err;
extern volatile unsigned int tim4debug0;
extern volatile unsigned int tim4debug1;
extern volatile unsigned int tim4debug2;
extern volatile unsigned int tim4debug3;
extern volatile unsigned int tim4debug4;
extern volatile unsigned int tim4debug5;
extern volatile unsigned int tim4debug6;
extern volatile unsigned int tim4debug7;

extern volatile unsigned int tim4cyncnt;

static unsigned int tim4cycnt_prev;


unsigned int tim4debug2_prev = 0;
unsigned int tim4debug3_prev = 0;
unsigned int tim4debug4_prev = 0;
unsigned int tim4debug5_prev = 0;
unsigned int tim4debug6_prev = 0;



extern volatile u32 canlogct;
u32 canlogct_prev;


u32 canlogSDwritectr_prev;

static u16 onetime = 0;

u16 k_gps_ctr;
u16 monitor_gps_state;

/* ====================================================================== */

/* Subroutine prototypes */
void output_gps_cnt (unsigned int uiT);

/******************************************************************************
 * void p1_PC_monitor_gps(void);
 * @brief	: Output current GPS & tick ct to PC
 ******************************************************************************/


void p1_PC_monitor_gps(void)
{
	int diff0;
int occt = 0;
int ocmiss = 0;
int ocmiss_prev = 0;
int octmp = 0;
int ticksvcyncnt = 0;
int diff2;
int diff3;
int diff5;
int diff6;
u32 canlogct_diff;



	switch (monitor_gps_state)
	{
	case 0:
		onetime = 0; k_gps_ctr = 0;
		if (usMonHeader == 0)	// Check if a header has been placed
			monitor_gps_state = 1;	// Here, no.  Place a column header
		else
			monitor_gps_state = 2;	// Here, yes. Just put a line of data
		usMonHeader = 1;	// Only one header per menu selection
		break;

	case 1: /* Do a column header */
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
		USART1_txint_puts(" GPS time sync data, number of msgs per sec\n\r");
		USART1_txint_send();//  (@1)
		monitor_gps_state = 2;
		break;

	case 2:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full
		
		/* Some monitoring stuff for Tim4_pod_se.c debugging. */
		if (ticks_flg != ticks_flg_prev)	// One sec ticks
		{ // Here yes.
			ticks_flg_prev = ticks_flg;	// Update flag counter
			canlogct_diff = canlogct - canlogct_prev; // Number of logged msgs
			canlogct_prev = canlogct;	// For computing number of received msgs
			if (onetime++ > 4)
			{
				onetime = 5;
				ticksvcyncnt = (int)(ticks - (int)(tim4cyncnt-tim4cycnt_prev));
				if (ticksvcyncnt > cycntHI) cycntHI = ticksvcyncnt;
				if (ticksvcyncnt < cycntLO) cycntLO = ticksvcyncnt;
			}
			tim4cycnt_prev = tim4cyncnt;

			occt = (int)(tim4debug5 - tim4debug5_prev);
			if (ticksvcyncnt < -32768) errctr += 1;
			octmp = (int)(tim4debug3-tim4debug3_prev);
			if ((octmp > 1282) || (octmp < 1279)) ocmiss += 1;

			if ((ocmiss - ocmiss_prev) != 0)
			{
				ocmiss_prev = ocmiss;
			}
			diff6 = (tim4debug6 - tim4debug6_prev); tim4debug6_prev = tim4debug6;
			diff5 = (tim4debug5 - tim4debug5_prev); tim4debug5_prev = tim4debug5;
			diff3 = (tim4debug3 - tim4debug3_prev); tim4debug3_prev = tim4debug3;
			diff2 = (tim4debug2 - tim4debug2_prev); tim4debug2_prev = tim4debug2;
	
			printf ("%5u ",++k_gps_ctr);	// Running counter
			printf ("%5d ", ticksvcyncnt); 	// ticks between time sync msgs
//printf ("%8u %8d %4u ",ticks, ticks_dev,tim4_tickspersec_err);
			printf ("%8u ", diff5);
//printf ("%6d %6d ", (int)tim4debug0, tim4debug1);
//printf ("%8d %8d %8d %8d %4u ", deviation_oneinterval, phasing_oneinterval, tim4debug4, diff6, tim4debug7);
//printf ("%4u %4u %1x ",tim4_64th_0_er, tim4_64th_19_er,tim4_readyforlogging);
//printf ("%5u %5u ", diff2, diff3 );
//printf ("%8u ", diff5);

			diff0 = ((int)canlogSDwritectr - (int)canlogSDwritectr_prev);
			printf ("%5u ",diff0); 		// Count of writes to SD card
			canlogSDwritectr_prev = canlogSDwritectr;

			printf ("%5u ",canlogct_diff); // Number of CAN msgs received

			printf("\n\r");
			USART1_txint_send();		// Start line sending.

			tim4debug4_prev = tim4debug4;
		}

		break;
	}

	return;
}
/*****************************************************************************************
Setup an int as a seconds and tick within second
*****************************************************************************************/

void output_gps_cnt (unsigned int uiT)
{
	printf (" %8u %4u ",(uiT >> ALR_INC_ORDER),(uiT & ( (1<<ALR_INC_ORDER)-1) ) ) ;	// Setup the output

	return;
}



