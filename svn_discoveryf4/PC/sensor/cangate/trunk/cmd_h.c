/******************************************************************************
* Date First Issued  : 02/06/2014
* Board              : PC
* Description        : histogram readout from shaft sensor
*******************************************************************************/
/*

02-07-2014 rev 193: Poll repsonse for shaft sensor

*/
#include "cmd_h.h"
#include "cmd_q.h"
#include "USB_PC_gateway.h"
#include "parse.h"
#include "parse_print.h"
#include "winch_merge2.h"	// struct CANCAL, plus some subroutines of use

#define NOPRINTF	1	// parse.c: 0 = printf, not zero = don't printf

/* Subroutine prototypes */
static void cmd_h_sendmsg(struct CANRCVBUF* p);

/* From cangate */
extern FILE *fpList;	// canldr.c File with paths and program names
extern int fdp;		// File descriptor for input file

/* List of program specs */
#define NUMPROGS	64	// Number of load specs
static struct LDPROGSPEC  ldp[NUMPROGS];
//static int loadpathfileIdx = 0; // Index in program number
//static int loadpathfileMax = 0; // Index+1 of last program number

/* List of CAN IDs extracted from input file. */
#define SIZECANID	512	// Max number array will hold
static struct CANIDETAL canidetal[SIZECANID];
static int idsize;	// Number stored in the foregoing array.

#define BUFSIZE	512
static char buf[BUFSIZE];	// Keyboard entry line buffer

static u32 keybrd_id;	// Unit ID
static u32 mode;	// 0 = histogram, 1 ADC3,2 filtered readings
static int state = 0;
static struct CANRCVBUF canmsg1;
static struct CANRCVBUF canmsg2;
static struct CANRCVBUF canmsg3;
static struct CANRCVBUF canmsg4;
static struct CANRCVBUF canmsg5;
static u8 canseqnumber = 0;
static struct PCTOGATEWAY pctogateway; 

static u32 cmd_h_bin;
static u32 cmd_h_binsv;
static u32 ctr = 0;
static u32 ctr2 = 0;
static u32 errct = 0;
static u32 timer = 0;
static u32 errtimoutct = 0;
static u32 errsumct = 0;
static u32 xmitctr = 0;

#define ADCLARGESTBIN	64

struct HISTO
{
	u32 total;		// Total count rcv'd
	u32 err;
	u32 sum;
	u32 bin[ADCLARGESTBIN+1];	// Bins rcv'd
}histo3,histo2;

struct TWO
{
	int i;
	int j;
};

struct TWO two;

/* Index of parameter, calibrations, CAN IDs */
#define NUMSTRS	5	// Number slots to look up

/* Sub-Strings ind description field of parameters, calibrations, and CAN IDs in .txt file. */
static const char *pstr[NUMSTRS] = {"ADC3 HistogramA","ADC3 HistogramB","ADC2 HistogramA","ADC2 HistogramB","ADC3 ADC2 readings"};
static int ssi[NUMSTRS];	// Index for slot for above descriptions fields that match strings
static unsigned int canid[NUMSTRS];

/******************************************************************************
 * static int lookup_index(struct LDPROGSPEC *p, int j, char *pc);
 * @brief 	: Look index for matching string in description field for subsystem
 * @param	: p =pointer to struct of unit that contains the subsystem
 * @param	: j = index in stuct of unit for the subsystem of interest
 * @param	: pc = pointer to char string to match against description 
 * @return	: 0 or +: index where there is a match
 *		: -1 = no match found
 *		: -2 = more than one match found
 *		: -3 = nothing to match
*******************************************************************************/
static int lookup_index(struct LDPROGSPEC *p, int j, const char *pc)
{
	int i;
	int numsubslots = p->slotidx;	// Total number of slots for this unit
	struct SLOTSTUFF *pstart = &p->slot[0]; // Convenience pointer
	int m = -1;

//printf("numsubslots: %u\n",numsubslots);
	/* Look up index for description that matches 'pc'. */
	for (i = 0; i < numsubslots; i++)
	{
		if ( strstr(&pstart->description[0], pc) != NULL)
		{
			if (m >= 0) 
			{
				printf(" Error: more than one match\n");
printf("slot description found: %i, %i %s %s %s\n", i, j, &pstart->format[0], &pstart->description[0], pc);
					return -2;	// Error: Already have a match
			}
			m = i;	// Save slot index
			printf("'%s' slot description found: index = %i  type code = %i .txt file line number: %i\n %s = 0x%08X @%s\n", 
				pc, i, pstart->type, pstart->linenumber, &pstart->format[0], pstart->x, &pstart->description[0]);
		} 
		pstart++;
	}

	if (m < 0)
	{
		printf(" Error: no match\n");
		 return -1; // Error: no match
	}

	return m; // Success.  Return index for the match
}

/******************************************************************************
 * static struct TWO lookup_subsystem(struct LDPROGSPEC *p, int idsize, char *pc);
 * @brief 	: Look up the subsystem
 * @param	: p = pointer to struct array holding input file data
 * @param	: idsize = size of array
 * @param	: pc = pointer to name to match
 * @return	: 0 or + = success and index to array
 *		: -1 = error
*******************************************************************************/
static struct TWO lookup_subsystem(struct LDPROGSPEC *p, int idsize, char *pc)
{
	int i,j;
//	int sz;
	struct TWO too = {-1, -1}; // Default, fail codes.

	/* Search array for this subsystem name. */
	for (i = 0; i < idsize; i++) // Unit level
	{
		for (j = 0; j < p[i].subsysnum; j++) // Subsystem level
		{
			if (p->subsysnum > 0) // Skip if no subsystems
			{
//printf("%s\n",&p[i].subsys[j].name[0]);
				if (strstr( &p[i].subsys[j].name[0], pc) != NULL)
//				if (strncmp(&p[i].subsys[j].name[0],pc,sz) == 0)
				{ // Here, name was found.
					too.i = i; too.j = j;
					return too; // Return index of array
				}
			}	
		}
	}
	return too;	// Name was not found.
}

/******************************************************************************
 * static char cmd_h_menu(void);
 * @brief 	: 
 * @param	: 
 * @return	: char entered
*******************************************************************************/
static char cmd_h_menu(void)
{
	int i = 0;
	char *p;

		printf("Enter name of subsystem for this shaft encoder\n"
		" command q option 8 lists the subsystems\n"
		"enter name exactly as shown : ");
		i = 0;
		while (i == 0)
		i = read (STDIN_FILENO, buf, BUFSIZE);	// Read one or more chars from keyboard
		printf("The name entered by the Hapless Op was: \n%s\n",buf);
		p = strchr(buf,'\n');
	if (p != NULL) *p = 0;
	return 0;
}

/******************************************************************************
 * static void cmd_h_histozero(struct HISTO* p);
 * @brief 	: Reset 
 * @param	: p = pointer struct with data
*******************************************************************************/
static void cmd_h_histozero(struct HISTO* p)
{	
	int i;
	p->total = 1;
	for (i = 0; i < ADCLARGESTBIN; i++) p->bin[i] = 0;
	return;
}
/******************************************************************************
 * static void cmd_h_histoaddbin(struct HISTO* p, u32 bin, u32 count);
 * @brief 	: place reading in array
 * @param	: p = pointer struct with data
*******************************************************************************/
static void cmd_h_histoaddbin(struct HISTO* p, u32 bin, u32 count)
{	
	if (bin >= ADCLARGESTBIN) {bin = ADCLARGESTBIN; p->err += 1; return;}
	p->bin[bin] = count;
	return;
}
/******************************************************************************
 * static void cmd_h_histobinchk(struct HISTO* p);
 * @brief 	: place reading in array
 * @param	: p = pointer struct with data
*******************************************************************************/
static void cmd_h_histobinchk(struct HISTO* p)
{
	int i; 
	p->sum = 0;
	for (i = 0; i < ADCLARGESTBIN; i++) p->sum += p->bin[i];
	if ((p->total * 64) != p->sum) errsumct += 1;
	return;
}
/******************************************************************************
 * int cmd_h_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
int cmd_h_init(char* p)
{
	if (strlen(p) < 4)
	{ // Here too few chars
		printf("Too few chars for the 'h' command plus option, example\n"
		"h 0\n"
		"h <mode>\n"
		"  mode 0 = HISTOGRAM\n"
		"  mode 1 = ADC3 ADC2 filtered readings\n");
		return -1;
	}
	
	sscanf( (p+1), "%u",&mode);
	printf ("%u, MODE: ",mode);
	if (mode == 0) printf("HISTOGRAM\n");
	if (mode == 1) printf("ADC3 ADC2 filtered readings\n");
	if (mode > 1) {printf("mode must be 0 or 1\n"); return -1;}

	/*   .... */
	int ret;
	
	/* Build a list of CAN IDs */
	idsize = parse_buildcanidlist(fpList, canidetal, SIZECANID);
	if (idsize < 0){printf("EXIT: FILE CAN ID EXTRACTION ERROR when building CAN ID list from file: %d\n", idsize);return -1; }

	/* Get the list of path/file for the CAN bus unit programs */
	ret = parse(fpList, &ldp[0], NUMPROGS, canidetal,idsize,NOPRINTF);	// Read (or re-read) input file and build list	
	if ( ret < 0 ) {printf("EXIT: FILE PARSE ERROR when passing file the second time\n");return -1; }

	cmd_q_subsystems(&ldp[0], idsize);	// List subsystems names for the Op

	cmd_h_menu();	// Get name of subsystem

	/* Look up subsystem name. */
	two = lookup_subsystem(&ldp[0], idsize, buf);
	if (two.i < 0 ) 
	{ 
		printf ("Name was not found in subsystem list: %i %i\n", two.i, two.j);
		return -1;	//  Failed miserably.
	}

	/* Look up indices and CAN IDs for histogram. */
	int i;
	for (i = 0; i < NUMSTRS; i++)
	{
		printf("%2i: ",i+1);
		ssi[i] = lookup_index(&ldp[two.i], two.j, pstr[i]);
		if (ssi[i] < 0)
		{
			printf("Look up error\n"); return -1;
		}
		canid[i] = ldp[two.i].slot[ssi[i]].x;
//printf("canid: %08x %i\n",canid[i], ssi[i]);
		printf("\n");
	}

	state = 0;	// Sequence through the readout.

	/* Prepare 1st msg ADC3 */
	canmsg1.id = canid[0];
	canmsg1.dlc = 4;
	canmsg1.cd.ull = 0; // jic

	/* Prepare ADC3 bin readout command msg */
	canmsg2.id = canid[1];
	canmsg2.dlc = 0;
	canmsg2.cd.ull = 0; // Initial bin # is zero.

	/* Prepare 1st msg ADC2 */
	canmsg3.id = canid[2];
	canmsg3.dlc = 4;
	canmsg3.cd.ull = 0; // jic

	/* Prepare ADC2 bin readout command msg */
	canmsg4.id = canid[3];
	canmsg4.dlc = 0;
	canmsg4.cd.ull = 0; // Initial bin # is zero.

	/* Command ADC3 ADC2 filtered readings readout */
	canmsg5.id = canid[4];
	canmsg5.dlc = 0;
	canmsg5.cd.ull = 0; // jic

	ctr = 0; ctr2 = 0; errct = 0; timer = 0; errtimoutct = 0; errtimoutct = 0;
	cmd_h_histozero(&histo3); cmd_h_histozero(&histo2);

//printf("CANMSG1: %08x %08x %08x %08x\n",canmsg1.id, keybrd_id >> (u32)CAN_UNITID_SHIFT,CAN_DATAID29_1, (u32)CAN_DATAID29_1 >> CAN_DATAID_SHIFT);
	return 0;
}


/******************************************************************************
 * void cmd_h_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'h' command
 * @param	: 
*******************************************************************************/
/*

*/

void cmd_h_do_msg(struct CANRCVBUF* p)
{
  if (mode == 0)
  {
	switch(state)
	{
	case 0:
		cmd_h_sendmsg(&canmsg1);
		cmd_h_binsv = 0; cmd_h_bin = 0;
		state = 1;	
		printf("=====================================================\nEND CASE 1 CYCLE %u ERRCT %u TIMEOUT %u SUMCHK %u\n", ctr++, errct, errtimoutct, errsumct);
		break;
	case 1:
		if ((p->id & ~0x3) == (canmsg1.id & ~0x3)) 
		{
			printf("MSG 1: TOTAL %u\n", p->cd.ui[0]);
			histo3.total = p->cd.ui[0];
		}
		if ((p->id & ~0x3) == (canmsg2.id & ~0x3)) 
		{
			printf("MSG 2: %3u %3u %8u\n",cmd_h_bin, p->cd.ui[1],p->cd.ui[0]);
			cmd_h_histoaddbin(&histo3, p->cd.ui[1], p->cd.ui[0]);
			if (cmd_h_bin != p->cd.ui[1]) errct += 1;
			cmd_h_bin += 1;
			cmd_h_binsv = p->cd.ui[1];
			timer = 0;
		}

		if ((cmd_h_bin >= ADCLARGESTBIN) || (cmd_h_binsv >= ADCLARGESTBIN))
		{
			cmd_h_sendmsg(&canmsg3);
			cmd_h_binsv = 0; cmd_h_bin = 0; timer = 0;
			state = 2;	
			cmd_h_histobinchk(&histo3);
			printf("END CASE 0 CYCLE %u ERRCT %u TIMEOUT %u SUMCHK %u\n", ctr++, errct, errtimoutct, errsumct);
		}
		timer += 1;
		if (timer > 350)
		{
			cmd_h_sendmsg(&canmsg1); timer = 0; errtimoutct += 1;
		}
		break;

	case 2:
		if ((p->id & ~0x3) == (canmsg3.id & ~0x3)) 
		{
			printf("MSG 3: TOTAL %u\n", p->cd.ui[0]);
			histo2.total = p->cd.ui[0];
		}
		if ((p->id & ~0x3) == (canmsg4.id & ~0x3)) 
		{
			printf("MSG 3: %3u %3u %8u\n",cmd_h_bin, p->cd.ui[1],p->cd.ui[0]);
			if (cmd_h_bin != p->cd.ui[1]) errct += 1;
			cmd_h_histoaddbin(&histo2, p->cd.ui[1], p->cd.ui[0]);
			cmd_h_bin += 1;
			cmd_h_binsv = p->cd.ui[1];
			timer = 0;
		}
		if ((cmd_h_bin >= ADCLARGESTBIN) || (cmd_h_binsv >= ADCLARGESTBIN))
		{
			cmd_h_histobinchk(&histo3);
			state = 0;
		}
		timer += 1;
		if (timer > 350)
		{
			cmd_h_sendmsg(&canmsg3); timer = 0; errtimoutct += 1;

		}
		break;		

	}
	return;
  }
  else
  { // Here, mode ==1,  ADC3 ADC2 filtered readings until command cancelled
	switch (state)
	{
	case 0: 
		cmd_h_sendmsg(&canmsg5);
		state = 1;
		break;
	case 1:
		if ((p->id & ~0x3) == (canmsg5.id & ~0x3)) 
		{
			if (p->dlc != 8) errct += 1;
			printf("%6u %3u %5u %5u\n", ctr++, errct, p->cd.ui[0], p->cd.ui[1]);
		}
		if (ctr2++ > 2048)
		{
			ctr2 = 0; 
			state = 0;
		}
		break;
	}
  }
  return;
}
/******************************************************************************
 * static void cmd_h_sendmsg(struct CANRCVBUF* p);
 * @brief 	: Send the msg
 * @param	: p = Pointer struct with msg
*******************************************************************************/
static void cmd_h_sendmsg(struct CANRCVBUF* p)
{
	p->cd.ui[0] = xmitctr;
	pctogateway.mode_link = MODE_LINK;		// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;		// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, p); 	// Send to file descriptor (e.g. serial port)
printf("MODE: %u XMT%2u: %08x TO: %u XMITCTR: %u\n",mode, state,p->id,errtimoutct, xmitctr);
	xmitctr += 1;
	return;
}

