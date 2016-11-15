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
	char vv[256];
	char* pc;
	int i;

	/* The following detects 1 second demarcation has occured. */
	if (p->id != CAN_TIMESYNC1) return;

	i = (p->cd.ull & 0x3f);
	if (i == 0) // Check the 1/64th sec ticks in the time payload
	{
		uitemp = (  (p->cd.ull >> 6) );
	
		/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
		uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

		/* Reconvert to ascii (should match the above ascii where appropriate) */
		sprintf (&vv[1],"LOCAL %s", ctime((const time_t*)&uitemp));
		pc = strchr(&vv[1],'\n'); *pc = ' '; vv[0] = '\n';
		printf("%s",vv);
	}

	if ( ((p->cd.ull & 0x80) == 0) )
	{ // 
		printf(" .");
	}
	else
	{
		if ((p->cd.ull & 0x40) == 0)
			printf("%2i", (int)(p->cd.ull & 0x3f)/10);			
		else
			printf("%2i", (int)(p->cd.ull & 0x3f) % 10 );
	}



	
	


	return;
}
