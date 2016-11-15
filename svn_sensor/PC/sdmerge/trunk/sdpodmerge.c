/************************************************************************************
sdpodmerge.c - Merge lines in a file sorted on the linux*64 time
06-25-2014
*************************************************************************************/
/*
Expect two files that have been concantenated and sorted--
1) *RMX file with converted POD data
2) .WCH file with converted winch SD logger data
Merge the two lines into single lines.

./spodmerge <path/name input> <path/name output> < (stdin with sorted RMX +.WCH file)

make && ./sdpodmerge ../idfiles/CANid_benchtest.txt outputtest.txt < ~/winch/instrumentation/Sun_Jun__8_20.44.41_2014.WCH

Test file 'x' that has some POD and winch data sorted together
make && ./sdpodmerge ../idfiles/CANid_benchtest.txt outputtest.txt < ~/winch/computed/140712/x

Output line--

1 - Time stamp in ticks (Linux time * 64)
2 - 17 POD fields   (see ../idfiles/PODfields.txt)
18 - n Winch fields (see ../idfiles/<file>, e.g. CANid_benchtest.txt)

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include "winch_merge2.h"

/* Buried routines */
void outputline(FILE* fp);
void pkt_tension(void);
void packet_print_date1(unsigned long long tt);
static void pkt_tension_init(void);

int CANidnotfound_err = 0;

#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits

#define MAXLINESZ_IN	512	// Input line buffer
char bufin[MAXLINESZ_IN];

#define PODFIELDCOUNT	16	// Number of fields in a POD output line
#define MAXLINESZ_OUT	1024	// Output line buffer
char bufout[MAXLINESZ_OUT];

unsigned long long ullPrevtime;
unsigned long long ullNewtime;
unsigned long long ullFirsttime;


char asciitime[64];		// Hold's linux time converted to ascii, extracted from tension

#define MAXPODBUFFSZ	128
char buff_pod[MAXPODBUFFSZ];
#define MAXWCHBUFFSZ	256
char buff_winch[MAXWCHBUFFSZ];

int linesin = 0;	// Total ct of input lines
int outlinect = 0;	// Total ct of output lines
int PODonlyct = 0;	// Total ct of POD data, no WCH data
int WCHonlyct = 0;	// Total ct of WCH data, no POD data
int BOTHct = 0;		// Total ct of POD and WCH both present
int noPODsw = 0;
int noWCHsw = 0;

/********************************************************************************
main
********************************************************************************/
int main (int argc, char **argv)
{
	int j;
	char* pc;
	char cC;
//	unsigned long long ullNew;
	FILE *fpCANid;
	FILE *fpOut;
	int ret;
	int sw_startup = 0;
	int bogustimect = 0;

	printf("\n> START: spodmerge.c\n");

	/* Setup files from command line arguments. */
	if (argc != 3){printf("Error %d arguments on command line.  Two arguments required: CAN id file and output file\n",argc); return -1;}

	if ( (fpCANid = fopen (*(argv+1),"r")) == NULL){printf ("\nCAN ID file did not open %s\n",*(argv+1)); return -1;}

	if ( (fpOut = fopen (*(argv+2),"w")) == NULL){printf ("\nOutput file did not open %s\n",*(argv+2)); return -1;}

	printf("CAN id file:       %s\n",*(argv+1));
	printf("Merge output file: %s\n",*(argv+2));

	/* Read in CAN ID file (with error checking) */
	ret = winch_merge_gettable(fpCANid);

	if (ret < 0) {printf("Got problems with CAN id file: %d\nReview the listing and fix it\n",ret); return -1;}
	printf("Sucess reading & checking the CAN id file\n");

	/* Print a summary of the cantable */
	winch_merge_printtable();

	/* List output fields usage. */
	winch_merge_printffields();
//printf("DONE\n");return 0; // Test readin to cantable

	winch_merge_init();	// Reset for next accumulating msgs for next time tick
	pkt_tension_init();

/* ====================== Read in lines piped to this routine ========================== */
	while ( (fgets (&bufin[0],MAXLINESZ_IN,stdin)) != NULL)	// Get a line from stdin
	{
		linesin += 1;	// Count input lines

		/* Get record ID number that is stored at the end of the line if it is RMX file */
		j = strlen(bufin);	// Find end of line
		//printf("%s\n",bufin);

		if (j < 26) continue;	// Line too short to be WCH or RMX

		/* Get time (1/64th secs) */
		sscanf(bufin,"%10llu",&ullNewtime);	// Time ticks

		/* Exclude lines with bogus times. */
		if (ullNewtime < 2500000000) 
		{		
			bogustimect += 1;
			continue;
		}



		if (ullNewtime != ullPrevtime) 	// Did we encounter a new time?
		{ // Yes, so output what we accumulated during the last time duration
			
			if (sw_startup < 1) // This count could be bigger to get rid of garbage lines
			{ // Here, a one-time start up.
				sw_startup += 1; 
				ullFirsttime = ullNewtime;	// Save for summary at end
				ullPrevtime = ullNewtime; 	// 
			}
			else
			{ 
				outputline(fpOut);		// Assemble output line from fields that were filled in
				/* intialize stuff for new duration */
				winch_merge_init();	// Reset for next accumulating msgs for next time tick
				pkt_tension_init();
				ullPrevtime = ullNewtime;
			}
		}

		if (j > 78)	// Line is long enough to be RMX
		{
			cC = bufin[j-4];		// Extract ID char
			if (cC == '6')
			{
				bufin[j-4] = 0;		// Truncate string at ID char
				pkt_tension();
			}
		}
		else
		{ // See if it is a SD line
			if ( (pc=strchr(bufin,'|')) == NULL ) continue;
			{
				ret = winch_merge_msg(pc);
				if (ret < 0)
				{
					CANidnotfound_err += 1;
				}
				noWCHsw = 1;
			}
		}
//printf("\nQUIT NOW\n");exit(0);
	}
	winch_merge_printsummary();	// Print counts for each CAN ID

	/* Summary of input. */
	winch_merge_print_id_counts();	// List of count of instances for ID's that are in the cantable
	printf("\n## END OF INPUT: spodmerge.c ##, \n\n");
	printf("  Number of  input    lines %7d\n",linesin);
	printf("  Number of output    lines %7d\n",outlinect);
	printf("  Number of POD only  lines %7d\n",PODonlyct);
	printf("  Number of WCH only  lines %7d\n",WCHonlyct);
	printf("  Number of BOTH good lines %7d\n",BOTHct);
	printf("  Number of lines where time field was too early: %d\n\n",bogustimect);	

	printf("  FIRST DATE/TIME: "); packet_print_date1(ullFirsttime); 
	printf("  LAST  DATE/TIME: "); packet_print_date1(ullNewtime); 

	return 0;
}
/* ************************************************************************************************************* */
/* void pkt_tension(void);											 */
/* Convert tension packet											 */
/* ************************************************************************************************************* */
void pkt_tension(void)
{
	char* p;
	int bsw = 1;
	char* pout = buff_pod;

	p = &bufin[10]; // Start at space followng time stamp
//printf("%s\n",p);
	/* Eliminate spaces in output field and separate with ',' */
	while (*p != 0)
	{
		if (*p == ' ')
		{
			if (bsw == 0)
			{
				*pout++ = ',';
				bsw = 1;
			}
		}
		else
		{
			bsw = 0;
			*pout++ = *p;
		}

		p++;
	}
	*pout = 0;	// Place termination in output line
	noPODsw = 1; 	// Show we got a POD line
	return;
}
/* ************************************************************************************************************* 
static void pkt_tension_init(void); // Make a POD data line with no data
   ************************************************************************************************************* */
static void pkt_tension_init(void)
{
	int i;
	char* p = buff_pod;
	for (i = 0; i < PODFIELDCOUNT; i++)
		*p++ = ','; // Field separator
	*p = 0;	// Termination
	noPODsw = 0;	// Set switch to show if we have POD data off
	return;
}
/* ************************************************************************************************************* */
/* void outputline(void); 										         */
/* ************************************************************************************************************* */
void outputline(FILE* fp)
{
	int len;

	/* Start with time stamp. */
	sprintf(bufout,"%10llu,",ullPrevtime);

	/* Add POD line to output */
	strcat(bufout,buff_pod);
	len = strlen(bufout);

	/* Add winch data to output buffer in a ready-to-go format. */
	winch_merge_outputline(&bufout[len], &bufout[MAXLINESZ_OUT]);

//printf("%s\n",bufout);
	fprintf(fp,"%s\n",bufout);

	outlinect += 1;	// Count number of output lines
	if ((noPODsw == 0) && (noWCHsw != 0)) WCHonlyct += 1;
	if ((noPODsw != 0) && (noWCHsw == 0)) PODonlyct += 1;
	if ((noPODsw != 0) && (noWCHsw != 0)) BOTHct += 1;


	return;
}
/*******************************************************************************
 * void packet_print_date1(unsigned long long tt);
 * @brief 	: printf the block with date/time/tick
 * @param	: tt = Linux time * 64
 * @return	: void
*******************************************************************************/
void packet_print_date1(unsigned long long tt)
{
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
	printf ("%s |%3u\n",vv, tick);	

	return;
}



