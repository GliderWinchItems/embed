/******************************************************************************
* File Name          : winch_merge2.c
* Date First Issued  : 06/25/2014
* Board              : any
* Description        : Combine CAN msgs into one line
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "winch_merge2.h"


/* Error counters */
unsigned int winch_merge_er1 = 0;	// 
unsigned int winch_merge_er2 = 0;	// 
unsigned int winch_merge_er3 = 0;	// 'type'
unsigned int winch_merge_er4 = 0;


#define MAXCANPAYTYPESZ	3	// Number of payload arrangement types implemented
#define MAXCANIDFORMAT	32	// Max number of chars for format field
#define MAXCANFIELDDESCRPT	40	// Max number chars for the field description

/* Field layout and calibration of a reading */
struct CANCAL
{
	int	dlc;		// Size of payload for this payload instance
	int	ixbegin;	// Byte number of first byte of payload field
	int 	ixend;		// Byte number of last byte of payload field
	int	sign;		// 1 = unsigned; -1 = signed
	int	fld;		// Field position in output line (-1 = skip)
	int	fix;		// fix/float: 0 = int, 1 = double
	int	iscale;		// Same as above, but for fix = 0
	int 	ioffset;
	double	scale;		// Multiple payload value by this number
	double	offset;		// calibration = ((float)payload * scale) - offset;
	char	format[MAXCANIDFORMAT]; // Format for converting calibrated reading to ascii
	char	c[MAXCANFIELDDESCRPT];	// Field description
};

#define MAXCANCALFIELDS	8		// Max number of calibrations in one CAN msg
#define MAXCANDESCRIPTION	64	// Max number of chars in description field
#define	MAXCDEFINE	40		// Max size of #define name

struct CANIDTABLE
{
	unsigned int	id;		// ID for this CAN msg
	int	n;			// Number of fields loaded
	int	ct;			// Count of instances for this id
	struct CANCAL cal[MAXCANCALFIELDS]; // Payload field layout
	char c[MAXCANDESCRIPTION];	// Description of this CAN msg
	char cdef[MAXCDEFINE];		// #define name

};

#define CANTABLESIZE	256	// Number CAN id's we can handle
static struct CANIDTABLE     cantable[CANTABLESIZE];	// Table with CAN ID and payload data
static struct CANIDTABLE* wcantableID[CANTABLESIZE];	// Sort index for CAN ID's
static struct CANIDTABLE* wcantableCT[CANTABLESIZE];	// Sort index for CAN ID's

struct CANIDNOTFOUND
{
	unsigned int id;
	int ct;
}canidnotfound[CANTABLESIZE];
static int idxidnotfound = 0;	// Index into not found list

#define MAXOUTSTRSIZE	48	// Max output string size for one field
struct STR
{
	char s[MAXOUTSTRSIZE];
};

/* Array of strings is filled with fields as msgs come in */
#define MAXWINCHFIELDS	32	// Max number of fields in winch output line
struct STR outfield[MAXWINCHFIELDS];

static int idxoutfield;	// Highest output field number so far
static int idxcantable;	// Highest can table index so far

/* Buried routines */
static int printfpay1errors(struct CANCAL *pt, int linect);
static int getfielddescription(struct CANCAL *pt, char * p, int linect);
static int getformat(struct CANCAL *pt, char * p, int linect);
static int getcalibfield(struct CANCAL *pt, char * p, int linect);
/*******************************************************************************
 * static int cmpfuncIDb (const void * a, const void * b);
 * @brief 	: Compare function for => bsearch <= (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncIDb (const void * a, const void * b)
{
	const long long *pA = (long long *)a;	// 'a' points to 'key'
	const struct CANIDTABLE *B = (struct CANIDTABLE *)b; // 'b' pts to sorted list
	struct CANIDTABLE* pB = *(struct CANIDTABLE **)B;

	/* long long needed to handle +1,0,-1 comparison. */
	long long aa = *pA;	// Search on this value
	long long bb = pB->id;	// 'cantable' id

	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}
/*******************************************************************************
 * static int cmpfuncID (const void * a, const void * b);
 * @brief 	: Compare function for => qsort <= (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncID (const void * a, const void * b)
{
	const struct CANIDTABLE *A = (struct CANIDTABLE *)a;
	const struct CANIDTABLE *B = (struct CANIDTABLE *)b;
	struct CANIDTABLE* pA = *(struct CANIDTABLE **)A;
	struct CANIDTABLE* pB = *(struct CANIDTABLE **)B;

	long long aa = pA->id;
	long long bb = pB->id;

	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}
/*******************************************************************************
 * static int cmpfuncCT (const void * a, const void * b);
 * @brief 	: Compare function for qsort (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncCT (const void * a, const void * b)
{
	const struct CANIDTABLE *A = (struct CANIDTABLE *)a;
	const struct CANIDTABLE *B = (struct CANIDTABLE *)b;
	struct CANIDTABLE* pA = *(struct CANIDTABLE **)A;
	struct CANIDTABLE* pB = *(struct CANIDTABLE **)B;

	long long aa = pA->ct;
	long long bb = pB->ct;

	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}
/*******************************************************************************
 * static int cmpfunc (const void * a, const void * b);
 * @brief 	: Compare function for qsort and bsearch (see man pages)--using direct access
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfunc (const void * a, const void * b)
{
	const struct CANIDTABLE *A = (struct CANIDTABLE *)a;
	const struct CANIDTABLE *B = (struct CANIDTABLE *)b;
	long long aa = (unsigned int)A->id;
	long long bb = (unsigned int)B->id;


	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}
/* ************************************************************************************************************* *
 * ascii/hex to binary conversions (little endian)
 ************************************************************************************************************* */
static unsigned char hexbin1(char* p)
{
	if ((*p >= '0') && (*p <= '9')) return (*p - '0');
	if ((*p >= 'A') && (*p <= 'F')) return (*p - 'A' + 10);
	if ((*p >= 'a') && (*p <= 'f')) return (*p - 'a' + 10);
	return 0;
}
static unsigned char hexbin2(char* p)
{
	unsigned char x = hexbin1(p);
	p += 1;
	return  ((x << 4) | hexbin1(p));
}
static unsigned short hexshort(char* p)
{
	unsigned short x = hexbin2(p);
	p += 2;
	return ( x | (hexbin2(p)  << 8) );
}
static unsigned int hexint(char* p)
{
	unsigned int x = hexshort(p);
	p += 4;
	return ( x  | ((unsigned int)hexshort(p) << 16) );
}
static unsigned long long hexlonglong(char *p)
{
	unsigned long long x = hexint(p);
	p += 8;
	return ( x | ((unsigned long long)hexint(p) << 32));

}
/*******************************************************************************
 * static unsigned long long hexvariable(char *p, int n, int sign);
 * @brief 	: Convert binary payload to unsigned long long
 * @param	: p = pointer to ascii/hex 
 * @param	: n = number of bytes of payload
 * @param	: sign = -1 for signed; +1 for unsigned
 * @return	: unsigned long long
*******************************************************************************/
static unsigned long long hexvariable(char *p, int n, int sign)
{
	union ULL
	{
		unsigned long long ull;
		unsigned char c[8];
	}x;
	int i;
	unsigned char y;

	x.ull = 0;
	for (i = 0; i < n; i++) // Build binary value
	{
		y = hexbin2(p); // Convert pair to one byte
		x.ull = (x.ull << 8) | y;
	}
	if (sign < 0) 
	{ // Here, the converted value is signed
		if ( (y & 0x80) != 0) // Check hi ord bit of hi ord byte
		{ // Carry high order bit into higher order bytes
			for (i = n; i < 8; i++)
				x.c[i] = 0xff;
		}
	}
	return x.ull; // Return as unsigned even if signed
}
/* ************************************************************************************************************* *
 * calibrate, convert binary to ascii output field, and save in output array
**************************************************************************************************************** */
/* Copy calibrated, ascii, into the output field position array that accumulates readings during a tick interval. */
static void calibcopy(struct CANCAL *pcal, char *p)
{
	if ((pcal->fld < MAXWINCHFIELDS) && (pcal->fld >= 0))
	{ // Here, the field position number won't get us into trouble
		strncpy(&outfield[pcal->fld].s[0],p,MAXOUTSTRSIZE); // Copy ascii reading into field postion
//if ((pcal->fld == 8) ||(pcal->fld == 9) ||(pcal->fld == 10) )
//printf("fld: %d |%s|\n",pcal->fld, p);
	}
	else
//printf("calibcopy: fld: %d len: %d %s\n",pcal->fld,(int)strlen(p), p);
		winch_merge_er1 += 1;	// This shouldn't happen if read-in checking is OK.
	return;
}
/* Signed int */ 
static int scalib(struct CANCAL *pcal, int n)
{ 
	char vv[MAXOUTSTRSIZE]; int ncal; double dcal; int ret;
	if (pcal->fix == 0)
	{ // Here, fixed point
		ncal = pcal->iscale * n + pcal->ioffset;
		ret = sprintf(vv, pcal->format, ncal);
//printf("scalib: %d %s\n",n,vv);
	}
	else
	{ // Here, floating point
		dcal = pcal->scale * n + pcal->offset;
		ret = sprintf(vv, pcal->format, dcal);		
	}
//printf("scalib: n: %d scale: %f dcal: %f vv|%s\n",n, pcal->scale, dcal, vv);
	calibcopy(pcal, vv); // Copy output into output line field array
	return ret;
}
/* Unsigned int */
static int ucalib(struct CANCAL *pcal, unsigned int n)
{ 
	char vv[MAXOUTSTRSIZE]; unsigned int ncal; double dcal; int ret;
	if (pcal->fix == 0)
	{
		ncal = pcal->iscale * n + pcal->ioffset;
		ret = sprintf(vv, pcal->format, ncal);
//if ((pcal->fld == 8) ||(pcal->fld == 9) ||(pcal->fld == 10) )
//printf("ucalib: %d %d %s %s\n",ret, n,pcal->format,vv);
	}
	else
	{
		dcal = pcal->scale * n + pcal->offset;
		ret = sprintf(vv, &pcal->format[0], dcal);
//printf("ulcalib: n: %x scale: %f offset: %f dcal: %f ret: %d  %s\n ",n,pcal->scale,pcal->offset, dcal, ret, vv);
	}
	calibcopy(pcal, vv);
	return ret;
}
/* Signed long long */
static int llcalib(struct CANCAL *pcal, long long n)
{
	char vv[MAXOUTSTRSIZE]; long long ncal; double dcal; int ret;
	if (pcal->fix == 0)
	{
		ncal = pcal->iscale * n + pcal->ioffset;
		ret = sprintf(vv, pcal->format, ncal);
	}
	else
	{
		dcal = pcal->scale * n + pcal->offset;
		ret = sprintf(vv, pcal->format, dcal);		
	}
	calibcopy(pcal, vv);
	return ret;
}
/* Unsigned long long */
static int ullcalib(struct CANCAL *pcal, unsigned long long n)
{
	char vv[MAXOUTSTRSIZE];  unsigned long long ncal; double dcal; int ret;
	if (pcal->fix == 0)
	{
		ncal = pcal->iscale * n + pcal->ioffset;
		ret = sprintf(vv, pcal->format, ncal);
	}
	else
	{
		dcal = pcal->scale * n + pcal->offset;
		ret = sprintf(vv, pcal->format, dcal);		
	}
	calibcopy(pcal, vv);
	return ret;
}

/*******************************************************************************
 * static ret hextime(struct CANCAL *pcal, char* p);
 * @brief 	: Convert binary time number to Linux 'ctime' string and store string in field position
 * @param	: pcal = pointer to payload field of cantable
 * @param	: p = Enter here with p pointing first byte of payload
 * @return	: none as yet
*******************************************************************************/
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits
static int hextime(struct CANCAL *pcal, char* p)
{
	int ret;
	char vv[256]; 
	unsigned long long ull = hexlonglong(p);
	time_t t = ull >> 6;	// Time in whole seconds

	/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
	t += PODTIMEEPOCH;		// Adjust for shifted epoch

	/* Convert linux time to ascii date|time */
	sprintf (vv,"%s", ctime((const time_t*)&t));

	/* Eliminate newline */
	char* pv = vv;
	while ((*pv != '\n') && (*pv != 0)) pv++;
	ret = sprintf (pv," %2d",(unsigned int)(ull & 63) );  // Add tick count
	
	/* Substitute ':' with ' ' (to make MATLAB'ers happy) */
	pv = vv;	
	do
	{
		pv = strchr(pv,':');
		if (pv != NULL) *pv = ' ';
	} while (pv != NULL);

//printf("hextime: %s\n",vv);

	calibcopy(pcal, vv); // Copy output into output line field array
	return ret;
}
/*******************************************************************************
 * static int fieldset(struct CANCAL *pcal, char* p);
 * @brief 	: Convert binary input number to ascii output and store string in field position
 * @param	: pcal = pointer to payload field of cantable
 * @param	: p = Enter here with p pointing first byte of payload
 * @return	: sprintf ret value (should be 1)
*******************************************************************************/
static int fieldset(struct CANCAL *pcal, char* p)
{
	int ret;
	int tmp;
	char vv[MAXOUTSTRSIZE];

	/* 3 bytes, 5 bytes, 7 bytes, and 8 bytes handled as long longs. */
	unsigned long long ullcal;
	long long llcal;

	/* 1 byte, 2 bytes, 4 bytes handled as ints. */
	unsigned int ucal;
	int ncal;

	if (pcal->ixbegin == 0)
	{ // Here a no-payload msg.  Copy text from format field
		strncpy(vv,&pcal->format[0],(MAXOUTSTRSIZE-1));
		calibcopy(pcal, vv); // Copy output into output line field array
		return (strlen(&pcal->format[0]));
	}

	/* Convert payload field to binary */
	tmp = pcal->ixend - pcal->ixbegin + 1; // Length payload field
	if ((tmp > 0) && (tmp <= 8) )
	{ // Convert to binary appropriate for sign
//printf("fieldset: tmp: %d sign: %d :%s\n",tmp,pcal->sign,p);
		switch (pcal->sign)
		{ 
		case -1: // signed payload field
			switch (tmp)
			{
			case 1: ncal   = (signed char) hexbin2 (p - 2 + pcal->ixbegin*2); ret = scalib(pcal, ncal); break;	
			case 2: ncal   = (signed short)hexshort(p - 2 + pcal->ixbegin*2); ret = scalib(pcal, ncal); break;	
			case 4: ncal   = (signed int)  hexint  (p - 2 + pcal->ixbegin*2); ret = scalib(pcal, ncal); break;	
			case 8: llcal  = (signed long long)hexlonglong((p - 2 + pcal->ixbegin*2));                  ret = llcalib(pcal, llcal); break;
			default: llcal = (signed long long)hexvariable((p - 2 + pcal->ixbegin*2), tmp, pcal->sign); ret = llcalib(pcal, llcal); break;
			}
			break;
		case 1:  // unsigned payload field
			switch (tmp)
			{
			case 1: ucal = hexbin2 (p - 2 + pcal->ixbegin*2); ret = ucalib(pcal, ucal); break;	
			case 2: ucal = hexshort(p - 2 + pcal->ixbegin*2); ret = ucalib(pcal, ucal); break;		
			case 4: ucal = hexint  (p - 2 + pcal->ixbegin*2); ret = ucalib(pcal, ucal); break;	
			case 8: ullcal = hexlonglong(p - 2 + pcal->ixbegin*2);                                       ret = ullcalib(pcal, ullcal);break;	
			default: ullcal = (signed long long)hexvariable((p - 2 + pcal->ixbegin*2), tmp, pcal->sign); ret = ullcalib(pcal, ullcal); break;
			}
			break;
		case 2: // Linux time conversion
			if (tmp == 8)
			{
				ret = hextime(pcal, p);
			}
			else
			{ // I don't think we should ever come here...
				printf("fieldset case 2 error: tmp: %d  sign: %d\n",tmp, pcal->sign);
			}
			break;
		}
	}
	else
	{
		winch_merge_er2 += 1;
	}
//if ((pcal->sign) == 1)
//printf("fieldset: tmp: %d ucal: %u ret: %d fix: %d %s\n",tmp,ucal,ret,pcal->fix,p-13);
//if ((pcal->sign) == -1)
//printf("fieldset: tmp: %d ncal: %u ret: %d fix: %d %s\n",tmp,ncal,ret,pcal->fix,p-13);

	return ret;
}
/* *************************************************************************************************************
 * void winch_merge_init(void);
 * @brief	: Reset things for next tick interval
**************************************************************************************************************** */
void winch_merge_init(void)
{
	int k;
	for (k = 0; k <= idxoutfield; k++)
		strcpy(&outfield[k].s[0],""); // Clear old value from field

	return;
}
/* *************************************************************************************************************
 * int winch_merge_msg(char* pc);
 * @brief	: Main passes a CAN msg to this routine
 * @brief	: Lookup id, convert payload and place in output field array
 * @param	: pc = pointer to beginning of CAN msg
 * @param	: size = something?
 * @return	:  0 = OK
 *		: -1 = payload size out of range
 *		: -2 = CAN id not found
**************************************************************************************************************** */
static void addidnotfound(unsigned int id)
{ // Add this id to the list, zero the counter
	canidnotfound[idxidnotfound].id = id; canidnotfound[idxidnotfound].ct = 0;	// Add id and zero count
	idxidnotfound += 1; if (idxidnotfound >= CANTABLESIZE) 
	{
		idxidnotfound -= 1;	// Prevent overflow
		printf("## ID NOT FOUND LIST IS OVERFLOWING ## %08x\n",id);
	}
	qsort(&canidnotfound, idxidnotfound, sizeof(struct CANIDNOTFOUND), cmpfunc);// Sort for bsearch'ing
	return;
}

int winch_merge_msg(char* pc)
{
	/* Enter here with pc pointing '|' of .WCH format line. */
	char* p = pc;
	unsigned int id;	
	unsigned char dlc;
	struct CANIDTABLE* ptbl;
	struct CANIDNOTFOUND* pnftbl;
	int dlcsw;

	int ix;
	int ret;

	p += 3;			// Skip sequence number
	id = hexint(p);		// Get ID
	p += 8;			// Point to DLC field		
	dlc = hexbin2(p);	// Get payload byte count
	if ((dlc < 0) || (dlc > 8)) {printf("DLC out-of-range %d for id: %08x\n",dlc, id); return -1;}
	p += 2;		// Point to first byte of payload

	/* Look up ID here */
	long long xx = id;
	ptbl = bsearch(&xx, wcantableID, idxcantable, sizeof(struct CANIDTABLE*), cmpfuncIDb);
	
	if (ptbl == NULL) // Check for 'id' not in 'cantable'
	{ // Here, id was not found in cantable.  Make a list of not-in-table id's
		if (idxidnotfound != 0)
		{ // One or more IDs in table
			long long llx = (unsigned int)id;	// Needed for bsearch cmpfunc
			/* Look up this ID in the table for ID's not found. */
			pnftbl = bsearch(&llx, canidnotfound,idxidnotfound, sizeof(struct CANIDNOTFOUND), cmpfunc);
			if (pnftbl != NULL)
				pnftbl->ct += 1;	// Count instances
			else
				addidnotfound(id);	// Add to table of not found ids
		}
		else
		{ // Add this id to the list, zero the counter
			addidnotfound(id);
		}
		return -2;	// CAN id not found
	}
	else
	{ // Here CAN was found.
		/* 'bsearch' returned a pointer into the sorted list of pointers (i.e. 'wcantable') */
		ptbl = *(struct CANIDTABLE**)(ptbl); // Get the pointer to 'cantable'

		ptbl->ct += 1; 	// Count instances for this id

		/* Go through payload fields for this CAN ID. */
		dlcsw = 0;
		for (ix = 0; ix < ptbl->n; ix++)
		{
			if (dlc == ptbl->cal[ix].dlc)
			{
				dlcsw = 1;
				ret = fieldset(&ptbl->cal[ix], p); // Call routine with p pointing first byte of payload
				if (ret < 0) printf("BOGUS: fieldset ret: %d  id: %08X field ix: %d\n",ret, id, ix);
			}
		}
		if (dlcsw == 0) printf("DLC in msg %d and no match in cantable, for CAN ID %08X\n",dlc,id);
	}

	return 0;
}
/*******************************************************************************
 * void winch_merge_printsummary(void);
 * @brief 	: Summary of things
 * @param	: fp = input file
*******************************************************************************/
void winch_merge_printsummary(void)
{
	int i;
	if (idxidnotfound == 0)
		printf("\nAll CAN IDs were in the table\n");
	else
	{
		printf("\nNUMBER OF CAN IDs encountered that are not in the table: %d, the list follows\n", idxidnotfound);
		for (i = 0; i < idxidnotfound; i++)
			printf("%3d %08X %9d\n",i+1,canidnotfound[i].id,canidnotfound[i].ct);
	}
	return;
}


/*******************************************************************************
 * void winch_merge_print_id_counts(void);
 * @brief 	: List ids in cantable and the counts for each
*******************************************************************************/
void winch_merge_print_id_counts(void)
{
	int i;
	/* Sort by count of instances for each CAN ID. */
	qsort(&wcantableCT, idxcantable, sizeof(struct CANIDTABLE*), cmpfuncCT);

	printf("\nCOUNT OF INSTANCES IN THE FILE FOR CAN ID'S IN THE TABLE\n");
	printf("SORTED by COUNT          SORTED by CAN ID\n");
	for (i = 0; i < idxcantable; i++)
	{
		printf("%3d %08X %9d  %08X %9d\n",i+1,wcantableCT[i]->id,wcantableCT[i]->ct,wcantableID[i]->id,wcantableID[i]->ct);
	}
	return;
}
/*******************************************************************************
 * int winch_merge_gettable(FILE* fp);
 * @brief 	: Read in flat file with CAN id versus many things and check for errors
 * @param	: fp = input file
 * @return	:  0 = OK
 *		: -1 = error
*******************************************************************************/
#define WINCHLINESZ	512
static char buf[WINCHLINESZ];

int winch_merge_gettable(FILE* fp)
{
	char* p;
	int idx = 0;
	int linect = 0;
	int i;
	int j;
	char* p1;
	int ret;
	struct CANCAL *pt;
	int ct;
	unsigned int id1;

	/* ---------- Initialize some things --------------------------------------------------- */
	// For checking for gaps 
//	for (i = 0; i < MAXWINCHFIELDS; i++) strcpy(&outfield[i].s[0],"GAP");
	
	for (i = 0; i < CANTABLESIZE; i++)
	{
		strcpy (&cantable[i].c[0],"none");	// Unit|sensor description
		for (j = 0; j < MAXCANCALFIELDS; j++)
		{
			strcpy (&cantable[i].cal[j].c[0],"none"); // Payload field description
			strcpy (&cantable[i].cal[j].format[0],"none"); // Payload format
			cantable[i].cal[j].fld = -999;
		}
	}

	/* --------- Read file ------------------------------------------------------------------ */
	idx = -1;
	/* Read flat file with CAN id, field position in output line, and calibration */
	while ( (fgets (&buf[0],WINCHLINESZ,fp)) != NULL)
	{
		linect += 1;
		if (buf[0] == '#')
		{ // Here, CAN ID with description
			/* Advance index in cantable */
			idx  += 1; if (idx >= CANTABLESIZE) {printf("## BOMBED ##: input file for CAN ID list exceeds the array size! %d\n",CANTABLESIZE); exit(-5);}

			/* Copy #define name to cantable. */
			p = &buf[7]; p1 = &cantable[idx].cdef[0]; j = 0;
			while(   ( (*p == ' ') || (*p == '\t'))  && (*p != 0) ) p++; // Spin forward to name
			while( (!( (*p != ' ') || (*p != '\t')) ) && (*p != '\n') && (*p != 0) && (j++ <= (MAXCDEFINE-1)) )  *p1++ = *p++;// Copy name
			*p1 = 0;	// jic terminator

			/* Get CAN id and copy description field */
			p = strchr(&buf[7],'x'); // Find position of x in 0x12341234 of id
			if (p == NULL) p = strchr(&buf[8],'X'); // Find position of x in 0X12341234 of id
			if (p == NULL) {printf("CAN ID TABLE: 'x' not found, line %d\n",linect); exit(-6);}
			p++; 			// Point to 1st char of hex id
			sscanf(p, "%x", &id1);	// Get ID (note that high order byte is first in this case)

			/* Check if the ID is duplicated. */
			for (i = 0; i < idx; i++)
			{
				if (id1 == cantable[i].id) {printf("CAN id %08X is already present with id at line %d\n",id1,linect); exit(-6);}
			}
			cantable[idx].id = id1; 		// CAN ID
			wcantableID[idx] = &cantable[idx];	// Pointer for sorted ID indexing into cantable
			wcantableCT[idx] = &cantable[idx];	// Pointer for sorted count indexing into cantable

			/* Copy CAN unit description */
			p = strchr(buf,'@'); 	// Find start of description
			if (p != NULL)
			{	// Copy description 
				p++; // Skip '@'
				p1 = &cantable[idx].c[0]; ct = 0;
				while ( (!((*p == 0) || (*p == '\n'))) && (ct++ <= (MAXCANDESCRIPTION-1)) )
					*p1++ = *p++;
				*p1 = 0;
			}
			cantable[idx].n  = 0; // Initialize for number payload fields defined (jic)
			cantable[idx].ct = 0; // Initialize count of instance for this id  (jic)
//printf("X1| %08X | %s\n",cantable[idx].id,buf);
			continue;	// We expect the next data line to be the payload field for this ID
		}

		/* Here, not a CAN ID data line */
		if ( (buf[0] == '/') && (buf[1] == '/') && (buf[2] == '$') ) // Is it a payload data line?
		{ // Here payload record
			pt = &cantable[idx].cal[cantable[idx].n]; // Convenience pointer
			ret = sscanf(&buf[3],"%d %d %d %d %d",&pt->dlc,&pt->ixbegin,&pt->ixend,&pt->fix,&pt->sign);

// Debug: looking for something bogus
int m,mm;
for (m = 0; m < idx+1; m++)
{
  for (mm = 0; mm < MAXCANCALFIELDS; mm++)
	if (cantable[m].cal[mm].fld > 14) {printf("OOOOH: %d %d %08X %d \n",m,mm,id1,cantable[m].cal[mm].fld); exit (-19);}
}

			if (ret != 5){printf("Payload error: 1st four values: got %d instead of 5, at line %d\n",ret,linect); exit(-4);}
			ret = printfpay1errors(pt,linect);	// Check & list any errors
			if (ret == 0)
			{ // Here, 'ixbegin' 'ixend' 'fix' 'sign' are within bounds, so presume the hapless Op succeeded.
				ret = getcalibfield(pt, &buf[3], linect);	// 'scale' 'offset' 'fld'
				if (ret < 0) {printf("cantable input error: getcalibfield: ret: %d at line number %d\n",ret, linect);exit(-1);}

				ret = getformat(pt, &buf[3], linect);		// Format to be used
				if (ret < 0) {printf("cantable input error: getformat: ret: %d at line number %d\n",ret, linect);exit(-2);}

				ret = getfielddescription(pt, &buf[3], linect);// Description for this reading
				if (ret < 0) {printf("cantable input error: getfielddescription: ret: %d at line number %d\n",ret, linect);exit(-3);}

				// Here.  Wow! Everything passed the checks.  Op must not be so hapless.
				cantable[idx].n += 1; // Advance the payload field index & count
				if (cantable[idx].n >= MAXCANCALFIELDS) 
				{ // Here, More payload field records than we have allowed for (Op was hapless after all).
					printf("Payload error: Number of payload lines %d: exceeds limit %d at line number %d\n",cantable[idx].n, MAXCANCALFIELDS, linect);
					cantable[idx].n = (MAXCANCALFIELDS-1); // Hold at index/count last position
				}
			}
		}
		continue; // Go get another input line
	}

	/* Here, we are at the end of the input file with the CAN ID/Payload descriptions. */
	idxcantable = idx + 1; // Number of CAN ID's in 'cantable'

	/* --------- SORT BY CAN ID ---------------------------------------------------------------------- */
//winch_merge_printtable(); // Debug: look at table before sort
	qsort(&wcantableID, idxcantable, sizeof(struct CANIDTABLE*), cmpfuncID);// Sort for bsearch'ing


	return 0;
}
/* *************************************************************************************************************
 * static int getcalibfield(struct CANCAL *pt, char * p, int linect);
 * @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	:  0 = OK;
 * 		: -1 = fixed pt specified and wrong number of values
 * 		: -2 = floating pt specified and wrong number of values
 * 		: -3 = Something other than fixed|float in the 'fix' variable
 * 		: -4 = Output field position has already been used
**************************************************************************************************************** */
static int getcalibfield(struct CANCAL *pt, char * p, int linect)
{
	int ret = 0;
	int a0,a1,a2,a3,a4; // Dummy vars
	if (pt->fix == 0)
	{ // Here, fixed pt
		ret = sscanf(p,"%d %d %d %d %d %d %d %d",&a0,&a1,&a2,&a3,&a4,&pt->iscale,&pt->ioffset,&pt->fld);
		if (ret != 8){ printf("Payload error: should have read 8, but read %d, (fix) 'iscale ioffset fld' at line %d\n",ret, linect); return -1;}
	}
	else
	{
		if (pt->fix == 1)
		{
			ret = sscanf(p,"%d %d %d %d %d %lf %lf %d",&a0,&a1,&a2,&a3,&a4,&pt->scale,&pt->offset,&pt->fld);
			if (ret != 8){ printf("Payload error: should have read 8, but read %d, (float) 'scale offset fld' at line %d\n",ret, linect); return -2;}
		}
		else {printf("Payload error: 'fix' %d was not 0 or 1, at line %d\n",pt->fix, linect); return -2;}
	}
	if (pt->fld >= MAXWINCHFIELDS){printf("Payload error: output field number %d is out-of-bounds, at line %d\n",pt->fix, linect); return -3;}

	if (pt->fld > idxoutfield) idxoutfield = pt->fld; // Note highest output field encountered (0 - n)

	/* Check if this output field has been used. */
	int i,j;
	for (i = 0; i < CANTABLESIZE; i++) // Go through CAN ID array
	{
		for (j = 0; j < MAXCANCALFIELDS; j++) // And all the calibration fields for each CAN ID
		{
			if ((cantable[i].cal[j].fld == pt->fld) && (pt->fld != -1))
			{ // Here, field is not the one we just added
				if (&cantable[i].cal[j].fld != &pt->fld)
         {printf("Payload error: output field number error: payload field = %d at line %d has been already used with CAN id %08X, payfield index %d\n",pt->fld,linect,cantable[i].id,j); return -4;}
			}
		}
	}
	return 0;
}
/* *************************************************************************************************************
 * static int getformat(struct CANCAL *pt, char * p, int linect);
 * @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	:  0 = OK; greater
 * 		: -1 = 1st '"' not found
 *		: -2 = 2nd '"' not found
 *		: -3 = No chars in format
 *		: -4 = '%' not found
 *		: -5 = More than one '%'
 *		: -6 = Too many chars in format 
**************************************************************************************************************** */
static int getformat(struct CANCAL *pt, char * p, int linect)
{
	char* pp;
	char* p3;
	int tmp;
	
	/* Find start of format */
	p = strchr(p, '"');

	/* Check for reseanablness */
	if (p == NULL){printf("Payload format err: 1st quote not found at line %d\n",linect); return -1;}
	pp = strchr(p+1,'"');
	if (pp == NULL){printf("Payload format err: 2nd quote not found at line %d\n",linect); return -2;}
	if ((pp - p - 1) <= 0){printf("Payload format err: No chars %ld in format at line %d\n",(pp - p - 1),linect); return -3;}
	if ((pp - p - 1) >= (MAXCANIDFORMAT-1)){printf("Payload format err: Too many chars %ld in format at line %d\n",(pp - p - 1),linect); return -6;}
	if (pt->ixbegin != 0)
	{
		p3 = strchr(p,'%');
		if (p3 == NULL){printf("Payload format err: no '%%' in format at line %d\n",linect); return -4;}
		p3 = strchr(p3+1,'%');
		if (p3 != NULL){printf("Payload format err: more than one'%%' in format at line %d\n",linect); return -5;}
	}

	/* Copy format to cantable */
	p++; // Skip 1st '"'
	pp = strchr(p+1, '"');
	tmp = (pp-p);
	if (tmp >= (MAXCANIDFORMAT-1)) tmp = (MAXCANIDFORMAT-1);
	strncpy(pt->format, p, tmp);
	*(pt->format+tmp) = 0;
//printf("FORMAT: %d %s %s\n",(pp-p),p, pt->format);

	return 0; // Success
}
/* *************************************************************************************************************
 * static int getfielddescription(struct CANCAL *pt, char * p, int linect);
 * @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	: 0 = OK; less than zero = very bad;
**************************************************************************************************************** */
static int getfielddescription(struct CANCAL *pt, char * p, int linect)
{
	char *p1;
	int ct;

	/* Copy CAN unit description */
	p = strchr(p,'@'); 	// Find start of description
	if (p == NULL)
	{
		printf("cantable intput error: getfielddescription: did not fined '@' at line number %d |%s\n",linect,p);
	 	return -1;
	}

	// Copy description 
	p++; // Skip '@'
	p1 = &pt->c[0]; ct = 0;
	while ( (!((*p == 0) || (*p == '\n'))) && (ct++ <= (MAXCANFIELDDESCRPT-1)) )
		*p1++ = *p++;
	*p1 = 0;

	return 0;
}
/* *************************************************************************************************************
 * static int printfpay1errors(struct CANCAL *pt, int linect);
 * @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	: 0 = OK; greater than zero = very bad;
**************************************************************************************************************** */
static int printfpay1errors(struct CANCAL *pt, int linect)
{
	int err = 0;
	if ((pt->dlc  > 8)    || (pt->dlc < 0))		{printf("Payload field error: dlc is out of rang : %d at line %d\n",pt->dlc,linect); err |= 1;}
	if ((pt->ixbegin > 8) || (pt->ixbegin < 0))	{printf("Payload field error: payload begin index: %d at line %d\n",pt->ixbegin,linect); err |= 2;}
	if ((pt->ixend   > 8) || (pt->ixend   < 0))	{printf("Payload field error: payload end   index: %d at line %d\n",pt->ixend,linect); err |= 4;}
	if  (pt->ixbegin > pt->ixend) 			{printf("Payload field error: payload begin index: %d greater than end index: %d at line %d\n",pt->ixbegin,pt->ixend,linect); err |=8;}
	if ((pt->fix  < -2) || (pt->fix > 1)) 		{printf("Payload field error: 'fix' %d is out of range at line %d\n",pt->fix,linect); err |= 16;}
	if ((pt->sign < -2) || (pt->fix > 2)) 		{printf("Payload field error: 'sign' %d is out of range at line %d\n",pt->fix,linect); err |= 32;}
	return err;
}
/* *************************************************************************************************************
 * char* winch_merge_outputline(char* p, char* pmax);
 * @brief	: Build the winch output from the payload field strings 
 * @param	: p = pointer to output buffer
 * @param	: pmax = pointer to end+1 of whatever output buffer is provided
 * @return	: pointer to string terminator
**************************************************************************************************************** */
char* winch_merge_outputline(char* p, char* pmax)
{
	int k;
	int len;
char* px = p;
	for (k = 0; k <= idxoutfield; k++)
	{
		len = (2 + strlen(&outfield[k].s[0]));  // Size of what are to add
		if ( (p + len) > pmax){printf("winch_merge_outputline: Insufficient output buffer\n"); continue;}
		strcpy(p,   &outfield[k].s[0]); //
		p += strlen(&outfield[k].s[0]);
		*p++ = ',';	// comma separate
	}
	*(p-1) = 0;	// Terminate the string	and overwrite trailing comma

	return p; // Return pointing to the string terminator
}
/* *************************************************************************************************************
 * void winch_merge_printtable(void);
 * @brief	: Nice listing of the cantable 
**************************************************************************************************************** */
void winch_merge_printtable(void)
{
	int i;
	int j;
	char s[32];
	struct CANCAL *pt;
	struct CANIDTABLE *pcan;
	int ct;		// Line number for payload field
	int idct = 1;	// CAN ID line numbering

	printf("\nCANTABLE: %d CAN ID entries. \n",idxcantable);
	for (i = 0; i < idxcantable; i++)
	{
		pcan = wcantableID[i];	// Get pointer from sorted array
		printf("--------------------------------------------------------\n%3d %08X %s %s \n",idct++,pcan->id, &pcan->cdef[0], &pcan->c[0]);
		ct = 1;
		for (j = 0; j < pcan->n; j++) // Payload fields for this CAN id
		{
			pt = &pcan->cal[j];
			strcpy(s,"ERROR sign");
			if (pt->sign ==  0) strcpy(s,"nopaylod");
			if (pt->sign ==  1) strcpy(s,"unsigned");
			if (pt->sign == -1) strcpy(s,"  signed");
			if (pt->sign ==  2) strcpy(s,"special Linux time * 64");
			if (pt->fix == 0)
			{ // Fixed pt
				printf("   %2d [%d]-[%d] %s	scale %2d	offset %2d	format %s	field: %2d	%s\n",ct,pt->ixbegin, pt->ixend, s, pt->iscale, pt->ioffset, &pt->format[0], pt->fld, &pt->c[0]);
			}
			else
			{ // Here floating pt
				printf("   %2d [%d]-[%d] %s	scale %le	offset %le	format %s	field: %2d	%s\n",ct,pt->ixbegin, pt->ixend, s, pt->scale, pt->offset, &pt->format[0], pt->fld, &pt->c[0]);
			}
			ct += 1;
		}
	}
}
/* *************************************************************************************************************
 * void winch_merge_printffields(void);
 * @brief	: Print payload field layout
**************************************************************************************************************** */
void winch_merge_printffields(void)
{
	int i,j,k;
	int sw;
	printf("\nOUTPUT FIELD POSITION LIST, %d fields used\n",idxoutfield+1);
	for (k = 0; k <= idxoutfield; k++)
	{
		/* See if this field number is in the cantable */
		sw = 0;
		for (i = 0; i < idxcantable; i++)
		{
			for (j = 0; j < cantable[i].n; j++) // Payload fields within this CAN id
			{
				if (cantable[i].cal[j].fld == k)
				{
					printf("%3d \t%08X \t%s \t%s\n",k+1, cantable[i].id, &cantable[i].cal[j].c[0], &cantable[i].cdef[0]);
					sw = 1;
				}
			}
		}
		if (sw == 0)
			printf("%3d ---------- This field was not specified -------------------\n",k);
	}
	return;
}

