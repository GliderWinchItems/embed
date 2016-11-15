/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_l.c
* Author	     : deh
* Date First Issued  : 09/20/2013
* Board              : PC
* Description        : List time from time sync msgs on CAN BUS
*******************************************************************************/

#include "cmd_l.h"
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits
/******************************************************************************
 * void cmd_l_datetime(struct CANRCVBUF* p);
 * @brief	: Format and print date time from time sync msg in readable form
 ******************************************************************************/
void cmd_l_datetime(struct CANRCVBUF* p)
{
	/* Linux format time.  Tick count shifted right 11 bits (2048) */
	time_t uitemp;

	/* The following detects 1 second demarcation has occured. */
	if (p->id != CAN_TIMESYNC1) return;
	
	if ((p->cd.ull & 0x3f) != 0) return; // Check the 1/64th sec ticks in the time payload

	printf (" %010llx ", p->cd.ull);

	uitemp = (  (p->cd.ull >> 6) );

	/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
	uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

	/* Reconvert to ascii (should match the above ascii where appropriate) */
	printf (" LOCAL %s", ctime((const time_t*)&uitemp));		


	return;
}
