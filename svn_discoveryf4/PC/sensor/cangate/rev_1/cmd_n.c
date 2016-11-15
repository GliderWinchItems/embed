/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_n.c
* Author	     : deh
* Date First Issued  : 09/20/2013
* Board              : PC
* Description        : Details for handling the 'n' command (list id's and msg ct/sec)
*******************************************************************************/
/*
This lifts & modifies some code from--
~/svn_sensor/sensor/co1_Olimex/trunk/p1_PC_monitor_can.c

*/

#include "cmd_n.h"
#include <time.h>

time_t t_prior;


static void cmd_n_count(u32 id);

#define CMDNTBLSIZE	32		// Number of unique ID|data_type allowed
static u32 idtbl[CMDNTBLSIZE];		// Table of message ids
static u32 cttbl[2][CMDNTBLSIZE];	// Two tables of counts associated with msg ids
static int idxLast = 0;			// Points to available unfilled entry.

static u32 idx_cmd_n_ct;
/* Index for double buffer that switches each second */
static u32 idx_cmd_n_ct;	// Index for double buffering can msg counts

/* 1 second timing versus counts between time messages */
static u32 timemode;	// 0 = timer, 1 = counts between time messages
/******************************************************************************
 * void cmd_n_init(int tmode);
 * @brief 	: Reset 
*******************************************************************************/
void cmd_n_init(tmode)
{
	int i;
	timemode = tmode;	// Set static variable with time mode for counts

	idxLast = 0;	// Reset end-of-table (no id's in table)
	for ( i = 0; i < CMDNTBLSIZE; i++) // Zero out count table
	{
		cttbl[0][i] = 0;	cttbl[1][i] = 0;
	}

	return;
}
/******************************************************************************
 * void cmd_n_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output current msg ID's ('n' command)
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'n' has been
turned on by the hapless Op typing 'n' as the first char and hitting return.
Command 'u' sets the mode to use the CAN time messages for making the 1 second
determination.
*/
void cmd_n_do_msg(struct CANRCVBUF* p)
{
	int i;
//	u8 *pc;
	int b;
	u32 totalct = 0;
	time_t t_now;
	time(&t_now);
	int sw = 0;
//printf("%08x\n",p->id);

	/* The following detects 1 second demarcation has occured. */
	if (timemode == 0)
	{ // Here, system timer is used
		if (t_now != t_prior)	
		{ // Here, this is a new second
			sw = 1;
		}
	}
	else
	{ // Here we are using CAN time messages to mark the 1 sec point
		if (p->id == CAN_TIMESYNC1)
		{
			if ((p->cd.ull & 0x3f) == 0) // Check the 1/64th sec ticks in the time payload
			{
				sw = 1;
			}
		}
	}

	if (sw == 1)
	{ // Here, this is a new second
		t_prior = t_now;
//printf("%08x\n",p->id);
		{ // Here, this starts the second
			b = (idx_cmd_n_ct & 0x01); // Index to buffer not currently being filled
			idx_cmd_n_ct += 1;		// Switch buffers

			/* Summary once per second. */
			// Here yes.  Output summary
			if (idxLast == 0)
			{ // Here, there were no id's found
				printf("No id's so far\n");
			}
			else
			{ // Here, the table has some entries.
				printf ("%3u: ",idxLast);
				for (i = 0; i < idxLast; i++)
				{ // List'em all
					printf("%08x",idtbl[i]);	// Print full id
//					pc = (u8*)&idtbl[i];	// Point to msg id (as received)
//					printf("%02x",(*(pc + 3))); 	// The upper two bytes hold the
//					printf("%02x",(*(pc + 2))); 	// data type of unit id
					printf(" %3u| ",cttbl[b][i]);	// count
					totalct += cttbl[b][i];
					cttbl[b][i] = 0;	// Zero out for next round
				}
			}
//			printf("%4u\n",totalct);

/* Add: byte ct and utilization */
/* NOTE: baud rate is hard coded, e.g. 230400. */
static unsigned int debug_inct_prev = 0;
extern unsigned int debug_inct;
double x = (debug_inct - debug_inct_prev)*10.0;
printf("%4u %5u %4.1f",totalct, (debug_inct - debug_inct_prev), (x*100)/230400 );
extern u32 err_seq;
extern u32 err_ascii;
printf(" %u %u\n",err_seq, err_ascii);
debug_inct_prev = debug_inct;

		}
	}
	cmd_n_count(p->id);	// Build table and count msgs

	return;
}
/******************************************************************************
 * static void cmd_n_count(u32 id);
 * @brief 	: Build table of id|data_types and counts
 * NOTE: this is entered under interrupt from 'can_log_buff' in 'can_log.c'
*******************************************************************************/
static void cmd_n_count(u32 id)
{
	u32 *p = &idtbl[0];
	int i = 0;
	int b;

	if (id == 0) return; // Debugging

	/* Get the current cttbl buffer index */
	b = (idx_cmd_n_ct & 0x01);

	/* Search table for this ID|data_type */
	while ( ( id != *p ) && (i < idxLast) ) {p++; i++;}

	if (i < idxLast) // Did we find the id in the table?
	{ // Here, yes, found.  Increment count for this ID|data_type
		cttbl[b][i] += 1;
	}
	else
	{ // Here, not found.  Add to table
		idtbl[i] = id; 	// Save the id
		cttbl[b][i] = 1;	// Start off with the 1st count
		/* Advance the idx to the next open table entry. */
		idxLast +=1; if (idxLast >= CMDNTBLSIZE) idxLast = CMDNTBLSIZE-1;
	}
	return;
}

