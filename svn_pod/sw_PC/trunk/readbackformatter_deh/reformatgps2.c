/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : reformatgps2.c
* Creater            : deh
* Date First Issued  : 08/11/2012
* Board              : Linux PC
* Description        : Merge accelerometer & tension data from *sorted* output of ratechangecic.c
*******************************************************************************/
/*

// 08/11/2012 example--
gcc reformatgps2.c -o reformatgps2 -lm && sudo ./ratechangecic < ../dateselect/120722.182240RS | sort | ./reformatgps2 


*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits

static int compare_int(const void *a, const void *b);

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];


/* Time stamps from packets */
unsigned long long ullAticktime64;		// Accelerometer: Ticktime from packet in 1/64ths sec
unsigned long long ullPticktime64;		// Pushbutton: Ticktime from packet in 1/64ths sec
unsigned long long ullLasttime64;		// Latest time
unsigned long long ullNewtime64;		// Packet just read time

/* Missing accelerometer data */
char ca[128] = {"   0.577      0.577      0.577      1.000  "};

/* Counters for types of packets encountered */
#define NUMPKTTYPES	7	// Number of packet types plus default error
int nPktct[NUMPKTTYPES];

/* Nice label for packet ID...a bit of fluff */
char * Pktname[NUMPKTTYPES]={"Tension","GPS tim","BattTmp","Pshbutt","Accelro","GPS fix","Default"};


/* Save the output then with the paired other arrives do the combined output */
#define SAVESIZE	128
struct SAVEDOUTPUT
{
	unsigned long long ull;
	int	ct;
	int 	missed;		// Count to check for missing pair
	char buf[SAVESIZE];	// Saved portion of output line
}strT, strG;
	
/* Subroutine prototypes */
void pkt_tension(void);
void pkt_accelerometer(void);
void ratechggps(void);
void outputpair(void);
void pairing (struct SAVEDOUTPUT * s);
void removenewline(char * p);
void pkt_pushbutton(void);

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k,l,m;
	int nTemp;
	char *p;
	struct tm *ptm;
	char cC;


/* ************************************************************************************************************ */
/* Read in lines formated with 'reformat.c' piped to 'sort' and piped to this routine                           */
/* ************************************************************************************************************ */
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		/* Get record ID number that is stored at the end of the line */
		j = strlen(buf);	// Find end of line
if (j < 3) continue;

		cC = buf[j-2];		// Look at ID position holding ID char

		switch (cC)	// Dispatch according the the record ID
		{
		case '1':
		case '4':
		case '6':
			sscanf(buf,"%10llu",&ullNewtime64);	// Time ticks
			if (ullNewtime64 != ullLasttime64)
			{
				outputpair();
			}
		}
		
//printf ("3%i# %s",j,buf);
		switch (cC)	// Dispatch according the the record ID
		{
		case '1': // Tension reformatted line size
			pkt_tension();
			nPktct[0] += 1;  // Count this type of packet
			break;
		
		case '2': // GPS time stamp versus Linux
			nPktct[1] += 1;  // Count this type of packet
			break;

		case '3': // Battery/temp 
			nPktct[2] += 1;  // Count this type of packet
			break;

		case '4': // Pushbutton
			pkt_pushbutton();
			nPktct[3] += 1;  // Count this type of packet
			break;

		case '5': // Accelerometer reformatted line size
			pkt_accelerometer();
			nPktct[4] += 1;  // Count this type of packet
			break;

		case '6': // GPS data
			ratechggps();
			nPktct[5] += 1;  // Count this type of packet
			break;

		default: // Some sort of error
			printf ("##### ID Error: %c :%s \n",cC,buf);
			nPktct[6] += 1;  // Count any non-packet lines
			break;
		}
	}
	/* Summary printouts */
	printf ("Missing T: %4u\n",strT.missed); // Missing pair cts
	printf ("Missing G: %4u\n",strG.missed);
	for (i = 0; i < NUMPKTTYPES; i++) 	// Counts of packet types
		printf ("Pkt type%2u %s %6u\n",i+1,Pktname[i],nPktct[i]);
	return 0;
}
/* ************************************************************************************************************* */
/* void pkt_tension(void);												 */
/* Convert tension packet											 */
/* ************************************************************************************************************* */
void pkt_tension(void)
{
/* Commmented out lines remain in case accelerometer data is to added */

#define AINSERT	19
//	char c[512];
	char * p;

	/* Get time (1/64th secs) */
	sscanf(buf,"%10llu",&strT.ull);	// Time ticks
	ullLasttime64 = strT.ull;	// Update the latest time
	
	/* Save time string */
//	strcpy (c, &buf[AINSERT]); // Save time string

	/* Edit time string to make it import into Excel */
//	p = &c[0];
	p = &buf[AINSERT];
	while (*p != 0)
	{
		if ((*p == ':') || (*p == '|') )
		{
			*p = ' ';
		}
		p++;
	}

	/* Insert acceleration data */
//	strcpy (&buf[AINSERT],ca); // Copy accelerometer readings

	/* Replace time string at end */
//	p = &buf[AINSERT+strlen(ca)];	// Point to position to append time
//	strcpy(p,c);	// Append time string to end of readings

	/* Pair the tension reading with the gps reading and output if paired */
	// void pairing (struct SAVEDOUTPUT * s, unsigned long long * ullT, int idx)
	pairing(&strT);

	return;
}
/* ************************************************************************************************************* */
/* void pkt_accelerometer(void);										 */
/* Convert tension packet											 */
/* ************************************************************************************************************* */
void pkt_accelerometer(void)
{
	/* Get time (1/64th secs) */
	sscanf(buf,"%10llu",&ullAticktime64);	// Time ticks
	ullLasttime64 = ullPticktime64;	// Update the latest time

	/* Extract readings */
	buf[55] = 0;	// Place a string terminator after readings
	strcpy (ca,&buf[12]); // Copy just the readings
//printf (" %s\n",ca);
	return;
}
/* ************************************************************************************************************* */
/* void ratechggps(void);											 */
/* Convert rate changed gps line										 */
/* ************************************************************************************************************* */
void ratechggps(void)
{
	/* Get time (1/64th secs) */
	sscanf(buf,"%10llu",&strG.ull);	// Time ticks
	ullLasttime64 = strG.ull;	// Update the latest time

	/* Pair the tension reading with the gps reading and output if paired */
	// void pairing (struct SAVEDOUTPUT * s, unsigned long long * ullT, int idx)
	pairing(&strG);
	
	return;		
}
/* ************************************************************************************************************* */
/* void outputpair(void);											 */
/* Output tension with paired gps										 */
/* ************************************************************************************************************* */
void outputpair(void)
{
	char c;

	/* Get rid of newline since will will be appending the gps fix data */
	removenewline(strT.buf);

	/* Get rid of newline since will will be appending the pushbutton flag */
	removenewline(strG.buf);

if (strT.ull != strG.ull)
{
//	printf ("@@@@ ERROR pairing: %10llu %10llu\n",strT.ull,strG.ull);
}

	/* Add flag when pushbutton is present */
	if (ullPticktime64 == strT.ull)
		c = '#';
	else
		c = '.';

// Hex output for location event in raw download data
//	ull64 = ullt64 +(ullEpoch << 6);
//unsigned long long ullEpoch = PODTIMEEPOCH;
//printf("%10llx ",(strT.ull << 5));

	/* Note: skip time field on strG output since it is the same as the time in strT.buf */
	printf ("%s%s%c\n",strT.buf,&strG.buf[11],c);	// Output the combined line
	
//printf ("%10llu %10llu\n",strT.ull, strG.ull);

	/* Reset pairing counters */
	strG.ct = 0;
	strT.ct = 0;

	return;
}
/* ************************************************************************************************************* */
/* void pair(void);												 */
/* Pair tension, gps fix, and pushbutton									 */
/* ************************************************************************************************************* */
void pairing (struct SAVEDOUTPUT * s)
{
	/* Save for pairing with other reading */
	strncpy (s->buf,&buf[0],SAVESIZE);	// Copy data (and don't overflow buffer)

	/* Count readings since last output */
	s->ct += 1;

	 // This reading did not pair with other the saved reading.
	if (s->ct > 1)	// Was there an unpaired reading?
	{ // Here yes.
//		printf ("##### ERROR: unpaired %5u %s\n",s->missed, s->buf);		
		s->missed += 1;	// Count errors
		return;
	}
	return;
}
/* ************************************************************************************************************* */
/* void pkt_pushbutton(void);											 */
/* Save PB time for flagging Tension/gps lines									 */
/* ************************************************************************************************************* */
void pkt_pushbutton(void)
{
	/* Get time (1/64th secs) */
	sscanf(buf,"%10llu",&ullPticktime64);	// Time ticks
	ullLasttime64 = ullPticktime64;	// Update the latest time
//printf("PB\n");
	return;

}
/* ************************************************************************************************************* */
/* void removenewline(char * p);										 */
/* Add PB flag to output       											 */
/* ************************************************************************************************************* */
void removenewline(char * p)
{
	char *pp;

	/* Remove newline at end of line */
	if ( (pp=strchr(p,'\n')) == NULL)
	{
//		printf("##### ERROR new line|%s|\n",p);
		return;
	}
	*pp = ' ';	// Replace new line with space
	return;
}
