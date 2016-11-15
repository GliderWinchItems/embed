/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : pp1_PC_monitor_can.c
* Author             : deh
* Date First Issued  : 03/26/2013
* Board              : Olimex CO, (USART2)
* Description        : Output current CAN data to PC
*******************************************************************************/
/*
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
#include "common.h"
#include "common_can.h"
#include "common_time.h"
#include "can_log.h"

extern volatile unsigned int ticks_flg;

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
	USART2_txint_putc(a);	

	a = otbl[(c & 0x0f)];
	USART2_txint_putc(a);	

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
	int i;
	struct CANRCVSTAMPEDBUF temp;
	struct CANRCVSTAMPEDBUF* p = monitor_canbuf_get();
	if (p == 0) return;	// Return: msg not ready.

	/* Best save it locally. */
	temp = *p;	
	

	/* Convert the mess to hex and send to PC */
	for (i = 5; i >= 0; i--)
		output(temp.U.c[i]);

	printf(" %04x %04x %08x %08x\n\r",temp.id,temp.dlc,temp.cd.ui[0],temp.cd.ui[1]);


//	USART2_txint_puts("\n\r");
	USART2_txint_send();
	
	return;
}
/******************************************************************************
 * void p1_PC_can_monitor_retrieve_id(void);
 * @brief 	: Output current msg ID's ('n' command)
*******************************************************************************/
static unsigned int ticks_flg_prev = 0;
#define	IDSIZE	12	// Number of IDs to summarize
static u32 id[IDSIZE];	// ID as received
static u16 ct[IDSIZE];	// Count during one second
static int idIdx = 0;	// Index for currently available slot


void p1_PC_can_monitor_retrieve_id(void)
{
	int 	i;
	char 	*pc;
	int 	sw;
	u32	temp;

	while ((temp = monitor_n_canbuf_get()) != 0)
	{ // Builddata for summary 
		sw = 0;
		for (i = 0; (i < idIdx) && (sw == 0); i++)
		{
			if (temp == id[i])
			{
				ct[i] += 1;
				sw = 1 ;
			}
		}
		if (sw == 0)	// Did we find the id in the table?
		{ // Here, no.  Add id to our table
			id[idIdx] = temp;
			ct[idIdx] = 1;
			/* Advance index in table of id and ct */
			idIdx += 1; if (idIdx >= IDSIZE) idIdx = IDSIZE -1;
		}
	}

	/* Summary once per second. */
	if (ticks_flg != ticks_flg_prev)	// One sec ticks
	{ // Here yes.  Output summary
		ticks_flg_prev = ticks_flg;	// Update flag counter
		if (idIdx == 0)
		{ // Here, there were no id's found during this last second
			USART2_txint_puts("No messages\n\r"); USART2_txint_send(); return;
		}
		printf ("%3u: ",idIdx);
		for (i = 0; i < idIdx; i++)
		{ // List'em all
			pc = (char*)&id[i];	// ID
			output(*(pc + 3)); 	// Just the high byte
			printf(" %3u| ",ct[i]);	// count
			id[i] = 0;		// Zero out for next round
			ct[i] = 0;		
		}
		USART2_txint_puts("\n\r");	USART2_txint_send();
		idIdx = 0;
	}
	return;
}

