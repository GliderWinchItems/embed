/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : p1_PC_monitor_can.c
* Author             : deh
* Date First Issued  : 03/26/2013
* Board              : Sensor CO
* Description        : Output current CAN data to PC ('n' and 'm' commands)
*******************************************************************************/
/*
05/07/2014 - mods for sensor board (USART1)

This routine "monitors" the current data, by outputting the readings in ASCII to the
PC.

There are two functions (with commands in ' ')--

  1) 'n' Retrieve the unit ID and data type for all the 
possible loggable buffers that contain data.
      This is done once per 'n' 'x' cycle.

  2) 'm' Send back, in ASCII, the CAN msgs that match a unit id and data type.
      This continues until the 'm' (toggles) it off, or 'x' command is executed.


*/

#include "p1_common.h"		// This has almost everything (for POD routines)
//#include "common.h"
#include "common_can.h"
#include "common_time.h"
#include "can_log.h"
//#include "gps_1pps_se.h"

extern volatile unsigned int ticks_flg;

/******************************************************************************
 * static int adv_index(int idx, int size)
 * @brief	: Format and print date time in readable form
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
 ******************************************************************************/
static int adv_index(int idx, int size)
{
	int localidx = idx;
	localidx += 1; if (localidx >= size) localidx = 0;
	return localidx;
}


/******************************************************************************
 * static void output(char c);
 * @brief 	: Output pair of hex chars 
 & @param	: Input byte
*******************************************************************************/
static const char otbl[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
static void output(char c)
{
	char a;

	a = otbl[((c >> 4) & 0x0f)];
	USART1_txint_putc(a);	

	a = otbl[(c & 0x0f)];
	USART1_txint_putc(a);	

	return;
}
/******************************************************************************
 * void p1_PC_can_monitor_msgs(void);
 * @brief 	: Output current data for an ID ('m' command)
*******************************************************************************/
/*
Note: line starts with size (int hex) and ends with 0x0a
*/
u8 monitor_can_state = 0;	// 
u8 monitor_can_state_n = 0;

void p1_PC_can_monitor_msgs(void)
{
#ifdef m_COMMAND_MIGHTBEUSEFUL
	int i;
	struct CANRCVSTAMPEDBUF temp;

	struct CANRCVSTAMPEDBUF* p = monitor_m_canbuf_get();
	if (p == 0) return;	// Return: msg not ready.

	/* Best save it locally. */
	temp = *p;	
	
	/* Convert the mess to hex and send to PC */
	for (i = 5; i >= 0; i--)
		output(temp.U.c[i]);

	printf(" %04x %04x %08x %08x",(unsigned int)temp.id,(unsigned int)temp.dlc,(unsigned int)temp.cd.ui[0],(unsigned int)temp.cd.ui[1]);
#endif
	printf(" m\n\r");
	USART1_txint_send();
	
	return;
}
/******************************************************************************
 * void p1_PC_can_monitor_retrieve_id(void);
 * @brief 	: Output current msg ID's ('n' command)
 * NOTE: This is polled via PC_handler from mainline
*******************************************************************************/
#define CMDNTBLSIZE	32		// Number of unique ID|data_type allowed
static u32 idtbl[CMDNTBLSIZE];		// Table of message ids
static u32 cttbl[2][CMDNTBLSIZE];	// Two tables of counts associated with msg ids
static int idxLast = 0;			// Points to available unfilled entry.

static u32 idx_cmd_n_ct_prev;
/* Index for double buffer that switches each second when GPS 1 PPS (ic) arrives. */
extern volatile u32 idx_cmd_n_ct;	// Index for double buffering can msg counts


void p1_PC_can_monitor_retrieve_id(void)
{
	int i;
//	char *pc;
	int b;
	u32 totalct = 0;

	/* The following detects if the GPS 1 PPS has occured. */
	if ( idx_cmd_n_ct_prev  == idx_cmd_n_ct) return;
	idx_cmd_n_ct_prev  = idx_cmd_n_ct;
	b = (idx_cmd_n_ct_prev + 1) & 0x01; // Index to buffer not currently being filled

	/* Summary once per second. */
	// Here yes.  Output summary
	if (idxLast == 0)
	{ // Here, there were no id's found
		USART1_txint_puts("No messages are being logged\n\r"); USART1_txint_send(); return;
	}
	printf ("%3u: ",idxLast);
	for (i = 0; i < idxLast; i++)
	{ // List'em all
		printf("%08x %3u| ",idtbl[i],cttbl[b][i]);
		totalct += cttbl[b][i];
		cttbl[b][i] = 0;	// Zero out for next round
	}
	printf("%4u\n\r",totalct);USART1_txint_send();

	return;
}
/******************************************************************************
 * void command_n_count(u32 id);
 * @brief 	: Build table of id|data_types and counts
 * NOTE: this is entered under interrupt from 'can_log_buff' in 'can_log.c'
*******************************************************************************/
/* Switch to show 'can_log.c' that the time is ready and logging can start */
extern u8	tim4_readyforlogging;
void command_n_count(u32 id)
{
	u32 *p = &idtbl[0];
	int i = 0;

	if (id == 0) return; // Debugging
	if (tim4_readyforlogging != 0x3) // Is gps time and syncing ready for logging?
		return;	// Skip until things settle down

	/* Get the current cttbl buffer index */
	u32 idxTim4 = idx_cmd_n_ct;
	while (idxTim4 != idx_cmd_n_ct) idxTim4 = idx_cmd_n_ct; // In case Tim4 interrupted
	idxTim4 = idxTim4 & 0x01; // idxTim4 increments upon each GPS 1 PPS.

	/* Search table for this ID|data_type */
	while ( ( id != *p ) && (i < idxLast) ) {p++; i++;}

	if (i < idxLast) // Did we find the id in the table?
	{ // Here, yes, found.  Increment count for this ID|data_type
/* TIM4 will flip 'idx_cmd_n_ct' 0,1,0...upon each 1 PPS.  In 'p1_PC_mointor_can.c' the
   cttbl not being incremented is used to makeup the printf output, afterwhich it is
   zero'ed. */
		cttbl[idxTim4][i] += 1;
	}
	else
	{ // Here, not found.  Add to table
		idtbl[i] = id; 	// Save the id
		cttbl[idxTim4][i] = 1;	// Start off with the 1st count
		/* Advance the idx to the next open table entry. */
		idxLast = adv_index(idxLast, CMDNTBLSIZE);
	}
	return;
}



