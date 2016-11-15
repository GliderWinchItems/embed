/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : packet_print.c
* Writer	     : deh
* Date First Issued  : 06/17/2013
* Board              : any
* Description        : Print svn_sensor packets
*******************************************************************************/
#include "packet_extract.h"
#include "packet_print.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


/*******************************************************************************
 * void packet_printB(struct PKTP *pp);
 * @brief 	: printf the packet
 * @param	: pblk--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_printB(struct PKTP *pp)
{
	int i;
	unsigned char *p = pp->p;
	for (i = 0; i < pp->ct; i++)
	{
		printf ("%02x",*p++);
	}
	packet_print_date1(pp->p);
	printf ("\n");
	return;
}
/*******************************************************************************
 * void packet_print(struct PKTP *pp);
 * @brief 	: printf the packet
 * @param	: pblk--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/
union CANDATA	cd;

void packet_print(struct PKTP *pp)
{
	struct CANRCVTIMBUF can;

	/* Move binary SD data into CAN struct */
	packet_convert(&can, pp);
//if (can.R.id == 0x41a00000) // Select a particular CAN id 
{
	
	printf("%016llx %08x %08x %08x %08x ",can.U.ull,can.R.id,can.R.dlc,can.R.cd.ui[0],can.R.cd.ui[1]);

	packet_print_date1(pp->p); // Convert time stamp to linux time and print date/time

//	packet_printB(pp);
	printf("\n");
}
	return;
}
/*******************************************************************************
 * void packet_print_hex(unsigned char *p);
 * @brief 	: printf the block in hex
 * @param	: p = pointer to packet (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_print_hex(unsigned char * p)
{
	int j;
	for (j = 0; j < 512; j++)
	{
		if ((j & 15) == 0) printf ("\n");
		printf ("%02x ",*p++);
	}
	return;
}
/*******************************************************************************
 * void packet_print_date1(unsigned char *p);
 * @brief 	: printf the block with date/time/tick
 * @param	: p = pointer to packet (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_print_date1(unsigned char *p)
{
	unsigned long long tt = *((unsigned long long *)p);	// Get linux time in 1/64th sec ticks
	time_t t = tt >> 6;	// Time in whole seconds
	char vv[256];
	char *pv = &vv[0];
	unsigned int tick = tt & 63;

	/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
	t += PODTIMEEPOCH;		// Adjust for shifted epoch

	/* Convert linux time to ascii date|time */
	sprintf (vv,"  %s", ctime((const time_t*)&t));

	/* Eliminate newline */
	while ((*pv != '\n') && (*pv != 0)) pv++;
	*pv = 0;

	/* Output date|time along with 1/64th sec tick ct */
	printf ("%s |%3u",vv, tick);	

	return;
}
/*******************************************************************************
 * void packet_changelines(unsigned char *p);
 * @brief 	: printf lines of the block where there is a change
 * @param	: p = pointer to packet (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_changelines(unsigned char *p)
{
	int sw_1st = 0;			// Difference found switch
	unsigned char old[16];		// Previous line for comparison
	unsigned char *pw;		// Working working working
	unsigned char *px = (p + 16);	// Beginning of line pointer
	unsigned char *py = &old[0];	// Previous line pointer
	unsigned char *pz = (p + 512);	// End pointer

	/* Print 1st line */
	while (p < px)
	{
		*py++ = *p;		// Save 1st line
		printf ("%02x ",*p++);	// Print 1st line
	}
	printf ("\n");

	/* Scan & print remaining lines */
	while (px < pz)	// Have we reached end of block?
	{ // Here, no.
		/* Check if next line is the same */
		 pw = px; px = px + 16; py = &old[0]; sw_1st = 0;
		while (pw < px)	// Have we reached the end of the line?
		{ // Here, no.
			if (*pw++ != *py++) // Is char on next line same as old line char?
			{ // Here, no.  Stop the comparison scan.
				sw_1st = 1; // Flag for printing.
				break;
			}
		}
		if (sw_1st != 0)	// Did the scan show the lines the same?
		{ // Here, no, the next line differs, so print it out.
			py = &old[0]; pw = px - 16;
			while (pw < px)
			{
				*py++ = *pw;		// Update prev line
				printf ("%02x ",*pw++);	// Print new line
			}
			printf ("\n");
		}
	}
	return;
}
