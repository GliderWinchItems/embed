/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : reformatgpsaccelmerge.c
* Creater            : deh
* Date First Issued  : 11/04/2012
* Board              : Linux PC
* Description        : Merge gps, accelerometer, tension data from rate changed *sorted* outputs
*******************************************************************************/
/*
11/04/2012 Hack of reformatgps2.c


// 11/04/2012 example--
gcc reformatgpsaccelmerge.c -o reformatgpsaccelmerge -lm && ./reformatgpsaccelmerge < ~/winch/download/121104/x
gcc reformatgpsaccelmerge.c -o reformatgpsaccelmerge -lm && ./reformatgpsaccelmerge 3  < ~/winch/download/121104/121104.234957RAS

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
}strT, strG, strA, strP;	// struct's for Tension, Gps, Accelerometer, Pushbutton

char asciitime[64];		// Hold's linux time converted to ascii, extracted from tension 

char cW;	// Type of output: '0' = tension only, etc.,..see 'outputpair'
int nGapct;	// Count time gaps detected.
	
/* Subroutine prototypes */
void pkt_tension(void);
void pkt_accelerometer(void);
void ratechggps(void);
void outputpair(void);
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

	if (argc == 2)
	{
		cW = *argv[1];
	}
	else cW = ' ';

	strP.buf[0] = ' ';
/* ************************************************************************************************************ */
/* Read in lines formated with 'reformat.c' piped to 'sort' and piped to this routine                           */
/* ************************************************************************************************************ */
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		/* Get record ID number that is stored at the end of the line */
		j = strlen(buf);	// Find end of line
//printf("%s\n",buf);
if (j < 9) continue;

		cC = buf[j-2];		// Extract ID char
		buf[j-2] = 0;		// Truncate string at ID char

		switch (cC)	// Dispatch according the the record ID (last char on the line)
		{
		case '1':	// tension
		case '4':	// pushbutton
		case '5':	// accelerometer
		case '6':	// gps
			sscanf(buf,"%10llu",&ullNewtime64);	// Time ticks
			if (ullNewtime64 != ullLasttime64)	// Did this input encounter a new time?
			{ // Here, yes.  Output what we have paired up.
				if (ullLasttime64 != 0)
					outputpair();
			}
			break;
		}
		
//printf ("3%i# %s",j,buf);// Debugging

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
	for (i = 0; i < NUMPKTTYPES; i++) 	// Counts of packet types
		printf ("Pkt type%2u %s %6u\n",i+1,Pktname[i],nPktct[i]);
	printf("Time gap count:%10u\n",nGapct);
	return 0;
}
/* ************************************************************************************************************* */
/* void pkt_tension(void);											 */
/* Convert tension packet											 */
/* ************************************************************************************************************* */
void pkt_tension(void)
{
#define AINSERT	19
	char * p;

	/* Get time (1/64th secs) */
	sscanf(buf,"%10llu",&strT.ull);	// Time ticks
	ullLasttime64 = strT.ull;	// Update the latest time
	

	/* Edit time string to make it import into Excel */
	p = &buf[AINSERT];
	while (*p != 0)
	{
		if ((*p == ':') || (*p == '|') )
		{
			*p = ' ';
		}
		p++;
	}
	/* Save for pairing with other reading */
	strncpy (&asciitime[0],&buf[19],29);		// This gets appended at the end of the output line
	asciitime[29] = 0;				// String terminator
	strncpy (&strT.buf[0],&buf[0],19);		// Copy data
	strT.buf[19] = 0;

//printf("T %s \n",strT.buf);

	return;
}
/* ************************************************************************************************************* */
/* void pkt_accelerometer(void);										 */
/* Convert tension packet											 */
/* ************************************************************************************************************* */
void pkt_accelerometer(void)
{
	/* Get time (1/64th secs) */
	sscanf(buf,"%10llu",&strA.ull);	// Time ticks
	ullLasttime64 = strA.ull;	// Update the latest time
	
	/* Save for pairing with other reading */
	strncpy (&strA.buf[0],&buf[11],SAVESIZE); // Copy just the readings from the input buffer

//printf("A %s \n",strA.buf);
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

	/* Save for pairing with other reading */
	strncpy (&strG.buf[0],&buf[11],SAVESIZE); // Copy just the readings from the input buffer

//printf("G %s \n",strG.buf);
	return;		
}
/* ************************************************************************************************************* */
/* void outputpair(void);											 */
/* Output tension with paired gps										 */
/* ************************************************************************************************************* */
void outputpair(void)
{
	char c;
	int x = 0;
	
	signed long long ullD = (ullNewtime64 - ullLasttime64);
	if (ullD > 1)
	{
		printf ("%10llu %10llu TIME GAP (1/64 ticks) = %10llu\n",ullNewtime64, ullLasttime64, ullD);
		nGapct += 1;

	}


	/* Determine how tension, gps, and accelerometer lines match up */
	if (ullLasttime64 == strG.ull) x  = 1;
	if (ullLasttime64 == strA.ull) x |= 2 ;
	if (ullLasttime64 == strT.ull)
	{ // Here, we have a tension reading that matches the 64th sec time tick
		switch (cW)	// Command line request one of the following: ('0','1','2','3') space ' ' is the default.
		{
		case ' ': // No command line parameter
			switch (x) // 
			{
			case 0:	// Tension only
				printf ("%s %c %s\n",strT.buf,strP.buf[0],asciitime);
				break;
			case 1: // Tension and GPS
				printf ("%s%s %c %s\n",strT.buf,strG.buf,strP.buf[0],asciitime);
				break;
			case 2: // Tension and Accelerometer
				printf ("%s%s %c %s\n",strT.buf,strA.buf,strP.buf[0],asciitime);
				break;
			case 3: // Tension, GPS, Accelerometer
				printf ("%s%s%s %c %s\n",strT.buf,strG.buf,strA.buf,strP.buf[0],asciitime);
				break;
			}
			break;
		case '0':// Tension only
				printf ("%s %c %s\n",strT.buf,strP.buf[0],asciitime);
				break;
		case '1': // Tension and GPS
				if ((x == 1) || (x == 3))
					printf ("%s%s %c %s\n",strT.buf,strG.buf,strP.buf[0],asciitime);
				break;
		case '2': // Tension and Accelerometer
				if ((x == 2) || (x == 3))				
					printf ("%s%s %c %s\n",strT.buf,strA.buf,strP.buf[0],asciitime);
				break;
		case '3': // Tension, GPS, Accelerometer
				if (x == 3)
					printf ("%s%s%s %c %s\n",strT.buf,strG.buf,strA.buf,strP.buf[0],asciitime);
				break;
		}
	}
	else
	{ //Here no tension so we dump all the others
		printf("%10llu TENSION MISS\n",ullLasttime64);
	}

	/* Reset */
	ullLasttime64 = ullNewtime64;
	strP.buf[0] = ' ';	// Reset flag for PB
	
	return;
}
/* ************************************************************************************************************* */
/* void pkt_pushbutton(void);											 */
/* Save PB time for flagging Tension/gps lines									 */
/* ************************************************************************************************************* */
void pkt_pushbutton(void)
{
	ullLasttime64 = strP.ull;	// Update the latest time
	strP.buf[0] = '*';		// Flag PB
	return;
}

