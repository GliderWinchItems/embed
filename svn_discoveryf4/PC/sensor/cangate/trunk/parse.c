/******************************************************************************
* File Name          : parse.c
* Date First Issued  : 11/22/2014
* Board              : PC
* Description        : Read CAN .txt file and place into strut(s)
*******************************************************************************/


#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include <string.h>
#include "common_fixedaddress.h"
#include "common_highflash.h"
#include "ctype.h"

#include "parse.h"

/* Holds one or two strings of text in quotes extracted from a line. */
#define MAXQUOTESTR1	256
struct QUOTESTR
{
	int flag1;	// Return: length of s1; 0 = no data; 
	int flag2;	// Return: length of s2; 0 = no data;
	char s1[MAXQUOTESTR1];	// Extracted string between first pair of "
	char s2[MAXQUOTESTR1];	// Extracted string between second pair of "
};
struct QUOTESTR quotestr;

/* Routine protos */
static int readcalvalue(struct QUOTESTR* p, char* pin, int numbertype, struct LDPROGSPEC* pspec);
static int getquotedfields(struct QUOTESTR* pout, char* pin);
static int checkrange_canid(int a);
static int checkrange_skip_unit(int a);
static int checkrange_skip_calib(int a);
static int checkrange_force(int a);
static int canidcheck(struct CANIDETAL pidlist[], int idx);
static void bombout(int n);


/* From cangate */
extern FILE *fpList;	// canldr.c File with paths and program names
extern int fdp;		// File descriptor for input file

// CAN ID list 
#define CANIDTBLSIZE	1024
struct CANIDETAL canidtbl[CANIDTBLSIZE];
int canidtblIdx = 0;

static 	int linenumber = 0;	// Input file line number

/******************************************************************************
 * int parse_buildcanidlist(FILE* fpList, struct CANIDETAL pidlist[], int idsize);
 * @brief 	: Load a struct array with CAN IDs from .txt file
 * @param	: fpList = pointer to file
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
 * @return	: 0 = nothing to do; 
 * 		: + = Success--Number of struct items loaded into the struct array
 *		: -1 = error.
 *		: -2 = error, again.
*******************************************************************************/
int parse_buildcanidlist(FILE* fpList, struct CANIDETAL pidlist[], int idsize)
{
	char parse_buf[CMDPLINESIZE];	// File input buffer
	char* p;
	char* pp;
	int i;

	/* Init and rebuild each time. */
	linenumber = 0;		// Reference for locating CAN IDs in file
	int idx = 0;		// Index into struct array for CAN IDs
	rewind (fpList);	// Start from beginning

	/* Read the whole file and extract all the CAN IDs from the #define lines. */	
	while ( (fgets (&parse_buf[0],CMDPLINESIZE, fpList)) != NULL)	// Get a line
	{	
		if (strncmp(parse_buf, "#define",7) == 0)
		{ // Here, #define line
			pidlist[idx].linenumber = linenumber;	// Save file line number for this CAN ID

			// Save #define name.  Copy to temp location
			p = &parse_buf[7]; pp = &pidlist[idx].name[0];*pp = 0;i = 0;
			while (isspace(*p)  != 0) p++; // Spin to next not-white char
			while ( !((*p == ' ') || (*p == '\t'))  && (i++ < CANIDNAMESIZE)) *pp++ = *p++;
			*pp = 0;

			// Extract CAN id in hex
			p = strchr(parse_buf,'x'); // lowercase 'x'
			if ((p != NULL) && (*(p-1) =='0'))
				sscanf ((p+1), "%x",&pidlist[idx].canid);
			else
			{
				pp = strchr(parse_buf,'X'); // uppercase X
				if ((pp != NULL) && (*(pp-1) =='0'))
					sscanf ((pp+1), "%x",&pidlist[idx].canid);
				else
				{printf("line: %d: '0x' or '0X' not found for can id\n", linenumber); continue;}
			}
//printf("%3d 0x%08X %s\n",linenumber,pidlist[idx].canid, &pidlist[idx].name[0]);

			// Get 'description' field.  If not present set string terminator.
			p = strchr(&parse_buf[3], '@');pp = &pidlist[idx].description[0];*pp = 0;
			if (p != NULL)
			{ // Here we are pointing to the '@'.  Copy text up to newline
				i = 0; p++; 
				while(!((*p == '\n') || (*p == 0))&&(i++ < CANIDDESCRIPTIONSIZE)) *pp++ = *p++; *pp = 0;
			}

			// Check for duplicates
			if (canidcheck(&pidlist[0],idx) != 0)
			{printf("FIX ERROR----\n"); return -1;}

			// Advance index
			idx += 1;
			if (idx >= idsize)
			{printf("NUMBER OF CAN IDs (%d) EXCEEDS SPACE PROVIDED (%d)/nFIX ERROR----\n",idx, idsize); return -1;}
		}
		linenumber += 1;
	}
//	printf("NUMBER OF CAN IDs: %i\n",idx);
	return idx;	// Return number of CAN IDs found.
}

/******************************************************************************
 * int parse(FILE* fpList, struct LDPROGSPEC pldp[], int size, struct CANIDETAL pidlist[], int idsize, int sw);
 * @brief 	: Load structs with data from .txt file
 * @param	: fpList = pointer to file
 * @param	: pldp = pointer to struct array that is populated with .txt file stuff
 * @param	: size = number of items in the struct array
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
 * @param	: sw = 0 include printf's; not zero = exclude printf's
 * @return	: 0 = nothing to do; 
 * 		: + = Success--Number of struct items loaded into the struct array
 *		: -1 = error.
 *		: -2 = error, again.
*******************************************************************************/
int parse(FILE* fpList, struct LDPROGSPEC pldp[], int size, struct CANIDETAL pidlist[], int idsize, int sw)
{
	struct LDPROGSPEC* punit = &pldp[0];
	char parse_buf[CMDPLINESIZE];	// File input buffer
	char* p;
	char* pp;
	int i;
	int j;
	int swfirst_unit = 0;		// One time switch when reading file
	int swfirst_sub = 0;	
	int tmp;
	char typesw = 0;
	unsigned int canid_txt;
	int numbertype;
	int idx;
	unsigned int uix;
	char* puix;
	char canid_nametmp[CANIDNAMESIZE];
	char description[CANIDDESCRIPTIONSIZE];
	int loadpathfileIdx = 0; // Index in program number
	int loadpathfileMax = 0; // Index+1 of last program number
	struct CANCAL *pt;
	struct CANIDETAL *papp;
	int ret;
	struct SUBSYS* psub = &pldp[0].subsys[0];
	int slotct;
	int slot;
	int prev_relidx_slot = 0;

	/* The CAN IDs are assumed to have been extracted into an array. */
	if (idsize <= 0)
	{ // Argh!
		printf("ERR: THE CAN ID FILE HAS NO ENTRIES (that are extracted from the #define lines) %d\n",idsize);
		bombout(-13);
	}
	rewind(fpList);	// Start at beginning of the file.
if (sw == 0){
printf("SIZEOF struct LDPROGSPEC %d\n",(unsigned int)sizeof(struct LDPROGSPEC));
}

/* Debug--list the file */
//if (sw == 0){
//while ( (fgets (&parse_buf[0],CMDPLINESIZE, fpList)) != NULL) printf("%s",parse_buf);
//rewind(fpList);	// Start at beginning of the file.
//}

	/* This routine can be entered many times, so don't depend on static vars to be  zero. */
	linenumber = 0;
	rewind(fpList);		// Start .txt file from beginning with each new p command
	loadpathfileIdx = 0;	// Rebuild the list
	canidtblIdx = 0; 	// Rebuild CAN ID list

	for (i = 0 ; i < size; i++)
	{
		pldp[i].idct = 0;	// CAN ID index
		pldp[i].slotidx = 0;	// Index into slot structs
		for (j = 0; j < NUMCANIDS; j++)
			pldp[i].c_app[j].canslotidx = 0; // Index into payload structs
		pldp[i].subsysct = 0;	// Index into subsytem structs
		pldp[i].subsysnum = 0;	// Number of subsytems on this unit
	}
	

	/* Read the whole file and extract things from lines of interest. */	
	while ( (fgets (&parse_buf[0],CMDPLINESIZE, fpList)) != NULL)	// Get a line
	{
		typesw = 'x';
		linenumber += 1;	// Count input lines to display for the hapless Op.
		tmp = strlen(parse_buf);
		if (tmp < 4) continue;	// Skip if line too short for anything of interest
		if (strncmp(parse_buf, "// " ,3) == 0) continue;	
		if (strncmp(parse_buf, "//\t",3) == 0) continue;	
		if (parse_buf[0] ==  ' ') continue;	
		if (strncmp(parse_buf, "//$ ",4) == 0) typesw = '$'; // Output conversion lines skipped
		if (strncmp(parse_buf, "//$\t",4) == 0) typesw = '$'; // Output conversion lines skipped
		if (strncmp(parse_buf, "//i ",4) == 0) typesw = 'i'; // Calibration,parameter, CANID line
		if (strncmp(parse_buf, "//C ",4) == 0) typesw = 'C'; // Command 's' line
		if (strncmp(parse_buf, "//T ",4) == 0) typesw = 'T'; // Command 's' line
		if (strncmp(parse_buf, "//U ",4) == 0) typesw = 'U'; // Subsystem 
		if (strncmp(parse_buf, "//U\t",4) == 0) typesw = 'U'; // Subsystem 
		if (strncmp(parse_buf, "//c",3) == 0) typesw = 'c'; // Comment line
		if (strncmp(parse_buf, "//v",3) == 0) typesw = 'v'; // Struct line

		
		// Get 'description' field if present
		p = strchr(&parse_buf[3], '@');pp = &description[0];*pp = 0;
		if (p != NULL)
		{ // Here we are pointing to the '@'.  Copy text up to temp
			i = 0; p++; 
			while(!((*p == '\n') || (*p == 0))&&(i++ < CANIDDESCRIPTIONSIZE)) *pp++ = *p++; *pp = 0;
		}
		

		if ( (tmp > 9) && (strncmp(parse_buf, "#define",7) == 0) )
		{ // Here, #define line
			// Save #define name.  Copy to temp location
			p = &parse_buf[7]; pp = &canid_nametmp[0];*pp = 0;i = 0;
			while (isspace(*p)  != 0) p++; // Spin to next not-white char
			while ( !((*p == ' ') || (*p == '\t'))  && (i++ < CANIDNAMESIZE)) *pp++ = *p++;
			*pp = 0;


			// Extract CAN id in hex
			p = strchr(parse_buf,'x'); // lowercase 'x'
			if ((p != NULL) && (*(p-1) =='0'))
				sscanf ((p+1), "%x",&canid_txt);
			else
			{
				pp = strchr(parse_buf,'X'); // uppercase X
				if ((pp != NULL) && (*(pp-1) =='0'))
					sscanf ((pp+1), "%x",&canid_txt);
				else
				{printf("line: %d: '0x' or '0X' not found for can id\n", linenumber); continue;}
			}

			// Here, got CAN ID, now determine if 'P' or 'I' (or neither)
			p = strchr(parse_buf,'/'); // Look for first '/'
			if (p == NULL) {printf("line: %d ER: #define with no '/' \n",linenumber); continue;}
			if (*(p+1) != '/') {printf("line: %d ER: #define missing pair: // \n",linenumber); continue;}
			if (*(p+3) == 'P') typesw = 'P';
			else 
			{
				if (*(p+3) == 'I') 
				{
					typesw = 'I'; 
					puix = (p+3); // Save for later
				}
				else 
					typesw = ' ';
			}
		}
		if ( (tmp > 20) && (strncmp(parse_buf, "//i ", 4) == 0) ) typesw = 'i'; // Calibration line
		if ( (tmp > 20) && (strncmp(parse_buf, "//i\t", 4) == 0) ) typesw = 'i'; // Calibration line
		
		switch (typesw) // Deal with details of line
		{ 
		case 'P': // Unit CAN ID with program path/file name etc.
			if (swfirst_unit == 0) // Skip advancing index the first time.
				swfirst_unit = 1; 
			else
			{
				loadpathfileIdx += 1;  if (loadpathfileIdx >= size) 
					{loadpathfileIdx = (size - 1); printf("Path/file: TOO MANY lines starting with P, line number: %d\n", linenumber);}	
			}

			// Convenience pointer to shorten up statements
			punit = &pldp[loadpathfileIdx];
			psub = &punit->subsys[punit->subsysct];

			// Make sure indices start at beginning.
			/* Reset indices for non-P lines that follow (i.e. CAN ID's and calibrations) */
			punit->idct = 0;	// Reset index for App CAN ids.
			punit->slotidx = 0;	// Reset index for App calibration, et al.
			punit->c_app[punit->slotidx].canslotidx = 0;	// Reset index for output display calibration
			swfirst_sub = 0;	// Subsystems first time sw
			prev_relidx_slot = 0;	// Relative index for subsystem slot counting, previous
			psub->relidx_slot = 0;	// Relative index for subsystem slot counting, current
			psub->relidx_canid = 0;	// Relative index for subsystem marking where CAN IDs start with new subsystem
			punit->subsysct = 0;	// Index into subsytem structs
			punit->subsysnum = 0;	// Number of subsytems on this unit
			
			/* Copy can id, name, description extracted above to local temp location. */
			punit->c_ldr.canid = canid_txt;	// Hex id
			strncpy(&punit->c_ldr.name[0], &canid_nametmp[0],CANIDNAMESIZE-1); // #define name
			strncpy(&punit->c_ldr.description[0], &description[0],CANIDDESCRIPTIONSIZE-1); // description text

			/* Copy path/file from input file line */
			pp = &parse_buf[9];
			while (*pp++ != 'P'); // Spin forward to 'P'
			while (((*pp == '\t') || (*pp == ' ')) && (*pp != 0)) pp++; // Spin forward to first non-white char
			p = &punit->loadpathfile[0]; 
			while( (*pp != ' ') && (*pp != '\t') && (*pp != '\n') && (*pp != '@') && (*pp != 0) ) 
			{
				if (sw == 0){printf("%c",*p);}
				*p++ = *pp++; 
			}
			*p = 0;

			/* Spin forward to '@' */
			pp = &parse_buf[2];
			pp = strchr(pp, '@'); 
			pp++;

			/* Extract add-on fields */
			pp = &parse_buf[19]; while (*pp++ != '*');
			sscanf(pp,"%d %d %d",&punit->force, &punit->skip_unit, &punit->skip_calib);
if (sw == 0){
printf("\n ###########################################################################################################################\n");
printf("line: %d index: %d path/file: %s\n",linenumber, loadpathfileIdx, &punit->loadpathfile[0]);
printf("line: %d   canid_ldr 0x%08X skip crc: %d  skip unit: %d skip calib: %d\n",linenumber,punit->c_ldr.canid,punit->force,punit->skip_unit,punit->skip_calib);
}

			/* Sanity check on values */
			tmp  = checkrange_canid(punit->c_ldr.canid);
			tmp |= checkrange_force(punit->force);
			tmp |= checkrange_skip_unit(punit->skip_unit);
			tmp |= checkrange_skip_calib(punit->skip_calib);
			if (tmp != 0) { printf("ERR: .txt file needs to be fixed\n"); return -2;}
			break;

		case 'I': // Application CAN ID lines
			// Convenience pointer to shorten up statements
			punit = &pldp[loadpathfileIdx];
			punit->c_app[punit->idct].refct = 0;	// Count of references to this CAN ID
			punit->c_app[punit->idct].canid = canid_txt;	// Store can id extracted earlier
			strncpy(&punit->c_app[punit->idct].name[0], &canid_nametmp[0],(CANIDNAMESIZE-1)); // #define name
			strncpy(&punit->c_app[punit->idct].description[0], &description[0],(CANIDDESCRIPTIONSIZE-1)); // description text
			
			// Get canid index and check that .txt index matches where we are storing it
			sscanf((puix+1),"%d", &uix);
			if (uix != (punit->idct - punit->subsys[punit->subsysct].relidx_canid))
			{
				printf("line: %d ERR: 'I' line, index doesn't match sequence: idx: %i seq: %i\n\t %s", linenumber, uix, punit->idct, parse_buf);
printf("uix: %d, punit->idct: %d, punit->subsys[punit->subsysct].relidx_canid: %d\n",uix, punit->idct, punit->subsys[punit->subsysct].relidx_canid);
			}

			pp = strchr(&parse_buf[2], '@'); 
			if (sw == 0){printf("line: %d     %d canid_app: %08X %s", linenumber, punit->idct, punit->c_app[punit->idct].canid, pp);}

			punit->idct += 1;
			if (punit->idct >= NUMCANIDS) 
				{ punit->idct -= 1; printf("line: %d TOO MANY app can ids\n",linenumber);}

			break;

		case 'i': // Calibration, parameter, et al. line.
			// Convenience pointer to shorten up statements
			punit = &pldp[loadpathfileIdx];
			psub = &punit->subsys[punit->subsysct];

			slotct = 0;

			// Read index into 4 byte slots, and type of number
			sscanf(&parse_buf[4], "%d %d", &idx, &numbertype);

			/* Check that idx is what input data shows.  Note: The sequence of values loaded have to match what the application 
                           expects, hence checking that the indices match is done.  */
			if ((punit->slotidx - prev_relidx_slot) != idx)
			{	printf("line: %d, store index (%d) not equal to index on input line (%d)\n",linenumber, punit->slotidx, idx);
				printf("FIX IT and rerun\n"); bombout(-8);
			}

			/* Extract fields (strings) enclosed in quotes (format field) */
			tmp = getquotedfields(&quotestr, &parse_buf[6]);	// Extract quoted field(s)
			punit->slot[punit->slotidx].format[0] = 0; // String terminator in case no quote on line
			if (tmp < 0){printf("line: %d, '//i ' line ERROR WITH GET QUOTED FIELD(S): %d\n",linenumber, tmp); bombout (-9);}

			/* Save linenumber (for easy reference later). */
			punit->slot[punit->slotidx].linenumber = linenumber;

			/* Save type. */
			punit->slot[punit->slotidx].type = numbertype;

			/* Save format field (which is CAN ID for type 7 slots). */
			strncpy(&punit->slot[punit->slotidx].format[0],&quotestr.s1[0],(FORMATSIZE-1));
			punit->slot[punit->slotidx].format[(FORMATSIZE-1)] = 0; // JIC some bozo made the CAN ID name too long.

			/* Save description field */
			strncpy(&punit->slot[punit->slotidx].description[0], &description[0],(CANIDDESCRIPTIONSIZE-1)); // description text			

			if ((numbertype >= 0) && (numbertype <= 5))
			{
				if (strlen(&quotestr.s1[0]) > FORMATSIZE) 
				{  printf("line: %d, format string of %d is more than %d chars allowed\n",linenumber,(int)strlen(&quotestr.s1[0]),FORMATSIZE); break;}
				strncpy(&punit->slot[punit->slotidx].format[0], &quotestr.s1[0],(FORMATSIZE-1)); // Save format string

				/* Read in value and store in the 4 byte slots */
				if (sw == 0){printf("line: %d\t\t%2d calib value: format \"%s\" ",linenumber,punit->slotidx,&punit->slot[punit->slotidx].format[0]);}

				// Here, store some number (int, unsigned, float, etc.) in 4 byte slots
				slotct = readcalvalue(&quotestr, &parse_buf[6], numbertype, &pldp[loadpathfileIdx]);
				if (slotct < 0)	{ printf("line: %d, error reading value ('readcalvalue' routine)\n",linenumber); bombout(-10);}
			}
			
			union VU {unsigned long long ull; unsigned int ui[2];}vu; 
			union VS {long long sll; unsigned int ui[2];}vs; 
			union VX {unsigned int ui; float f;}vx; vx.ui = punit->slot[punit->slotidx].x;
			union VD {unsigned int ui[2]; double d;}vd; 

			switch (numbertype)
			{ // 0 - 5 are printfs to check that the readin is correct.  6 & 7 do some real work.
			case 0: // unsigned int
			case 1: // signed int
				if (sw == 0){printf("u32 or s32: %d",punit->slot[punit->slotidx].x);}
				break;
			case 2: // unsigned long long
				vu.ui[0] = punit->slot[punit->slotidx].x;
				vu.ui[1] = punit->slot[punit->slotidx+1].x;
				if (sw == 0){printf("u64: %llu",vu.ull);}
				break;
			case 3: // signed long long
				vs.ui[0] = punit->slot[punit->slotidx].x;
				vs.ui[1] = punit->slot[punit->slotidx+1].x;
				if (sw == 0){printf("s64: %llu",vs.sll);}
				break;
			case 4: // float
				if (sw == 0){printf("float: %0.8f",vx.f);}
				break;
			case 5: // double
				vd.ui[0] = punit->slot[punit->slotidx].x;
				vd.ui[1] = punit->slot[punit->slotidx+1].x;
				if (sw == 0){printf("double: %0.15f",vd.d);}
				break;
			case 6: // string
				if (quotestr.flag2 > 0) // Check that 2nd quoted field has some chars
				{ // Store the string, (if there is enough space (4 chars per slot))
					if ( ( (quotestr.flag2/4) + 1 + punit->slotidx) > (NUMBEROF4BYTSLOTS-1) )
					{ printf("line:%d, ERR: QUOTED FIELD exceeds calibration array space: %s\n",linenumber, &quotestr.s2[0]);}
					else
					{
						for (i = 0; i < (quotestr.flag2/4) + 1; i++)
						{
							punit->slot[punit->slotidx+i].x = *((unsigned int*)&quotestr.s2[0] + i) ; // Point to calibration slot
						}

/* ASCII data that goes in calibration array */
if (sw == 0){
printf("ascii: \"%s\" ",(char*)&punit->slot[punit->slotidx].x);
/* Print description */
p = &parse_buf[9]; p=strchr(p,'@'); p++;p=strchr(p,'@');if (p != NULL) printf("\t%s",p);else printf("\n");
}

						slotct = ((quotestr.flag2 - 1)/4) + 1;	// Adv the slot index

					}
				}
				else
				{ // No data in 2nd quote field, so store a null string
					punit->slot[punit->slotidx].x = 0; // Null string
					punit->slotidx += 1;	// Advance index
					printf("line: %d, ERR: NUMBER TYPE IS STRING, but NO 2nd quote string found\n",linenumber);
				}
				break;
	
			case 7: // CAN ID.  Look up by name in table and store binary value in slot.
				if (quotestr.flag1 <= 0) // Check that 1st quoted field has some chars
				{
					printf("line %d, ERR: NUMBER TYPE IS CAN_ID, but no chars in the quotes\n",linenumber); bombout(-11);
				}	
				else
				{
					if (quotestr.flag1 >= CANIDNAMESIZE)
					{printf("line %d, ERR: NUMBER TYPE IS CAN_ID, but no chars in the quotes\n",linenumber); bombout(-12);}
					slot = 0;
					/* Look up CAN ID name in list */
					for (i = 0; i < idsize; i++)
					{
						if ((strncmp(&pidlist[i].name[0], &quotestr.s1[0],CANIDNAMESIZE)) != 0) continue;
						// Found! Store binary CAN ID in slot.
						punit->slot[punit->slotidx].x = pidlist[i].canid;
						pidlist[i].refct += 1; // Count references to this CAN ID
						slotct = 1;	// One and only one slot required for a CAN ID!
						slot = 1;
						break;
					}
					if (slot == 0)
					{
						printf("line %d, ERR: CAN_ID name on this line %s, is not in CAN_ID list\n%s\n",linenumber,&quotestr.s1[0],&parse_buf[0]);bombout(-14);
					}
				}
				break;

			default: // Bogus number type code
				printf("line: %d, '//i ' record, has out-of-range number-type: %d\n",linenumber, numbertype);
				break;

			}
			punit->slotidx += slotct;		// Advance index in array of unsigned ints
			psub->relidx_slot = punit->slotidx;	// Continuously save latest slot (index for later)

/* Print description */
if (sw == 0){
p = &parse_buf[9]; p=strchr(p,'@'); if (p != NULL) printf("\t%s",p);
}
	
			/* Check that the index advance in the above didn't hit the end. */
			if (punit->slotidx > (NUMBEROF4BYTSLOTS-1)) 
			{punit->slotidx = NUMBEROF4BYTSLOTS-2;
			printf("line: %d, ERR: EXCEEDED NUMBER OF CALIBRATION SLOTS\n",linenumber);}

			break;

		case '$':	// Output field conversion line (skip these for now)
			// Convenience pointers to shorten up statements
			punit = &pldp[loadpathfileIdx];		// Unit
			papp = &punit->c_app[punit->idct];	// CANs ID for unit
			pt = &papp->cancal[papp->canslotidx]; 	// Output calibration fields for a CAN IDs

			/* Extract field (string) enclosed in quotes (format field) */
			tmp = winch_merge_getformat(pt, &parse_buf[4], linenumber);
			if (tmp < 0) {printf("line: %d, ER: //$ quote field failed\n", linenumber); return -1;}

			ret = sscanf(&parse_buf[3],"%d %d %d %d %d",&pt->dlc,&pt->ixbegin,&pt->ixend,&pt->fix,&pt->sign);

			if (ret != 5){printf("Payload error: 1st four values: got %d instead of 5, at line %d\n",ret,linenumber); bombout(-4);}
			ret = winch_merge_printfpay1errors(pt,linenumber);	// Check & list any errors
			if (ret == 0)
			{ // Here, 'ixbegin' 'ixend' 'fix' 'sign' are within bounds, so presume the hapless Op succeeded.
				ret = getcalibfield(pt, &parse_buf[3], linenumber);			// 'scale' 'offset' 'fld'
				if (ret < 0) {printf("cantable input error: getcalibfield: ret: %d at line number %d\n",ret, linenumber);bombout(-1);}

				ret = winch_merge_getformat(pt, &parse_buf[3], linenumber);		// Format to be used
				if (ret < 0) {printf("cantable input error: getformat: ret: %d at line number %d\n",ret, linenumber);bombout(-2);}

				ret = winch_merge_getfielddescription(pt, &parse_buf[3], linenumber);	// Description for this reading
				if (ret < 0) {printf("cantable input error: getfielddescription: ret: %d at line number %d\n",ret, linenumber);bombout(-3);}

				// Here.  Wow! Everything passed the checks.  Op must not be so hapless.
				papp->canslotidx += 1; // Advance the payload field index & count
				if (papp->canslotidx >= MAXCANCALFIELDS) 
				{ // Here, More payload field records than we have allowed for (Op was hapless after all).
					printf("Payload error: Number of payload lines %d: exceeds limit %d at line number %d\n",papp->canslotidx, MAXCANCALFIELDS, linenumber);
					papp->canslotidx = (MAXCANCALFIELDS-1); // Hold at index/count last position
				}
			}
			break;

// Can this case really occur?
		case ' ':	// Line with "// " but no following recognized char.
			printf("line: %d ERR: line starts with #define, but not a // P or // I line\n",linenumber);
			break;

		case 'U': // New subsystem 
			/* Calibrations et al. are store sequentially in one array.  Each time a new subystem is encountered 
			   the index is saved.  That index is the beginning of the calibrations et al. for the new subsystem.
			*/
			// Convenience pointer to shorten up statements
			punit = &pldp[loadpathfileIdx];
			psub = &punit->subsys[punit->subsysct];

			prev_relidx_slot = psub->relidx_slot;

			// Count number of '//U' lines encountered, i.e. number of subsystems.
			punit->subsysnum += 1;	// Count number
if (sw == 0){printf("SUBSYSTEM NUMBER: %u %s\n",punit->subsysnum, &punit->c_ldr.name[0]);}


			if (swfirst_sub == 0)
			{ // Here, first time through
				swfirst_sub = 1;		

				punit->subsysct = 0;	// Subsystems within a unit (0 - (n-1))
			}
			else
			{ // Here, not first time through
				punit->subsysct += 1;	
				if (punit->subsysct >= NUMBEROFSUBSYS) 
				{
					printf("line %d, ERR: Subsystem count of %d exceeds limit of %d\n", linenumber, punit->subsysct, NUMBEROFSUBSYS);
					bombout(-14);
				}
			}

			// Convenience pointer to subsystem struct
			psub = &punit->subsys[punit->subsysct];

			// Save index that points to next available calibration slot for subsystem.
			psub->relidx_slot  = punit->slotidx;
			psub->relidx_canid = punit->idct;
	
			tmp = getquotedfields(&quotestr, &parse_buf[3]);	// Extract quoted field(s)
			if (tmp < 0)
			{ // Here, extracting quoted field error
				printf("line %d, ERR: Subsystem quoted field error: %d\n",linenumber, tmp);
				bombout(-15);
			}
			if (quotestr.flag1 <= 0)
			{ // Here, extracting quoted field error
				printf("line %d, ERR: Subsystem quoted field has no chars: %d\n",linenumber, quotestr.flag1);
				bombout(-16);
			}

			strncpy(&psub->name[0], &quotestr.s1[0],SUBSYSNAMESIZE-1);	// Copy subsystem name from input line
			psub->name[SUBSYSNAMESIZE-1] = 0;	// JIC

			/* Spin up to description field and copy, if present. */
			psub->description[0] = 0; // String terminator in case no description field
			pp = &parse_buf[3];while((*pp != '@') && (*pp != '\n') && (*pp != 0)) pp++;
			if (*pp++ == '@')
			{ // Here, we found a description, so copy it 
				strncpy(&psub->description[0], pp, CANIDDESCRIPTIONSIZE-1);
				p = strchr(&psub->description[0],'\n'); // Remove newline if present
				if (p != NULL)
				{
					*p = 0;
				}
			}

			/* Spin past the quoted field and get subsystem number. */
			pp = &parse_buf[3];while(*pp++ != '"');while(*pp++ != '"');
			sscanf(pp,"%u", &psub->number);	// Read in subsystem number

			/* Get the struct name that goes with this subsystem. */
			pp = strstr(&parse_buf[4],"// S");
			if (pp != NULL)
			{
				pp += 4;	// Step past the 'S'
				// Spin forward to non-white.
				while (isspace(*pp++) == 0) ;
				// Copy name, (don't include comment field)
				p = &psub->structname[0];
				while ( !((*pp == ' ')||(*pp == '\t')||(*pp == 0)||(*pp == '@')||(*pp == '\n')) && (p < &psub->structname[CANIDNAMESIZE-1]) ) 
				{
					*p++ = *pp++;
				}
				*p = 0; // String terminator
				if (sw == 0){printf("line: %d SUBSYSTEM STRUCT: %s\n",linenumber,&psub->structname[0]);}
			}
			else
			{
				printf("line: %d Subsystem: struct name not present\n", linenumber);
			}

			break;

		case 'C': // Line for command 's'
		case 'T': // Line for command 's'
			if (sw == 0){printf("line: %d not applicable, a command 's' line\n",linenumber);}
			break;

		case 'c': // Comment line
			break;

		case 'v': // 'struct' definition line
			break;

		case 'x': // Not classified line
			if (sw == 0){printf("line: %d, ERR(?): LINE NOT RECOGNIZED\n%s",linenumber, &parse_buf[0]);}
			break;
		}
	}
	/* Here, end of file */

	loadpathfileMax = loadpathfileIdx + 1; // Number of specs in list
	if (loadpathfileIdx == 0)
	{
		printf("ERR: No path/file entries found (i.e. lines starting with ""P ""\n");
		return -1;	
	}
	loadpathfileIdx = 0;

	if (sw == 0){printf("Number of units/srec's in file: %3d\n\n####### END .txt read & parse .txt file. ############\n\n",loadpathfileMax);}
	return loadpathfileMax;

}
/******************************************************************************
 * static int getquotedfields(struct QUOTESTR* pout, char* pin);
 * @brief 	: Extract up to two quote fields from a line)
 * @param	: pout = ptr to resulting stuff
 * @param	: pin = input line
 * return	: 0 = OK, -1 = quote count not even; -2 = no quotes
*******************************************************************************/
static int getquotedfields(struct QUOTESTR* pout, char* pin)
{
	int ct = 0;
	char* p = pin;
	char* pp;
	pout->flag1 = 0; pout->flag2 = 0;
	pout->s1[0] = 0; pout->s2[0] = 0;

	/* Count number of '"' */
	while (!((*p == '\n') || (*p == 0)))
	{
		if ( (pp=(strchr(p, '"'))) == NULL)
			break;
		ct += 1;
		p = pp+1; // Point to next char after '"'
	}
	if ((ct & 0x1) != 0)	// '"' should be in pairs
	{ printf("line: %d, NUMBER OF QUOTES IS NOT EVEN: %d\n", linenumber, ct);
	  return -1;
	}
	if (ct < 1)	// There should be at least one pair
	{ printf("line: %d, LOOKING FOR QUOTES, BUT NOTHING IN QUOTES\n", linenumber);
	  return -2;
	}
	pp = pin;	p = &pout->s1[0];
	if ((pp=(strchr(pp, '"'))) != NULL) // Locate first '"'
	{
		pp++; // Next char after '"'
		while ( (*pp != '"' ) && (pout->flag1 < (MAXQUOTESTR1-1)) ) {*p++ = *pp++; pout->flag1 += 1;} // Copy field
	}
	*p++ = 0; // Place termination and stop to next char after '"'

	if (ct > 2)	// For can type string ("//i ") there should be a 2nd pair.
	{
		pp += 1;	p = &pout->s2[0];
		if ((pp=(strchr(pp, '"'))) != NULL)
		{
			pp++; 
			while (*pp != '"' ) {*p++ = *pp++;pout->flag2 += 1;}
		}
	}
	*p = 0;	// Place termination
	pout->flag2 += 1;	// Termination char adds to slot count

//if (pout->flag2 < 1)
//  printf("line: %d, QUOTE CT: %d flag1: %d %s flag2: %d\n",linenumber,ct, pout->flag1, &pout->s1[0], pout->flag2);
//else
//  printf("line: %d, QUOTE CT: %d flag1: %d %s flag2: %d %s\n",linenumber,ct, pout->flag1, &pout->s1[0], pout->flag2,&pout->s2[0]);
	
	return 0;	// Return success code
}
/******************************************************************************
 * static int readcalvalue(struct QUOTESTR* p, char* pin, int numbertype, struct LDPROGSPEC* pspec);
 * @brief 	: Read the value field 
 * @param	: p = ptr to resulting stuff
 * @param	: pin = input line
 * @param	: numbertype = 0 - n 
 * return	:  number of 4 byte slots used, -1 = error
*******************************************************************************/
static int readcalvalue(struct QUOTESTR* p, char* pin, int numbertype, struct LDPROGSPEC* pspec)
{
	/* One of each type of format conversion */
	unsigned int 	typ0;
	signed int 	typ1;

	/* Unions to take care of "slots" being unsigned ints (u32) */
	union TYP2 {	// code 2: U64
		unsigned int ui[2];
		unsigned long long ull;
	}typ2;

	union TYP3 {	// code 3: S64
		unsigned int ui[2];
		long long sll;
	}typ3;	

	union TYP4 {	// code 4: float
		unsigned int ui;
		float f;
	}typ4;

	union TYP5 {	// code 5: double
		unsigned int ui[2];
		unsigned long long ull;
		double d;
	}typ5;	


	if (p->flag1 < 1)
	{ printf("line: %d ERR: 'readcalvalue' no chars in format field\n",linenumber); bombout(-1);}

	// Zero out old values jic
	typ1 = 0; typ2.ull = 0; typ3.sll = 0; typ4.ui = 0; typ5.ull = 0;

	while (*pin++ != '"'); while (*pin++ != '"');
	switch (numbertype)
	{ //             Pointers:    input   format    value 
	case  CALTYPE_U32:	sscanf(pin, &p->s1[0], &typ0);	
			pspec->slot[pspec->slotidx].x = typ0;		return 1;
		break;
	case  CALTYPE_S32:	sscanf(pin, &p->s1[0], &typ1); 
			pspec->slot[pspec->slotidx].x = typ1;		return 1;
		break;
	case  CALTYPE_U64:	sscanf(pin, &p->s1[0], &typ2.ull);
		pspec->slot[pspec->slotidx].x = typ2.ui[0]; 
			pspec->slot[pspec->slotidx+1].x = typ2.ui[1]; 	return 2;
		break;
	case  CALTYPE_S64:	sscanf(pin, &p->s1[0], &typ3.sll);
		pspec->slot[pspec->slotidx].x = typ3.ui[0]; 
			pspec->slot[pspec->slotidx+1].x = typ3.ui[1]; 	return 2;
		break;
	case  CALTYPE_FLOAT:	sscanf(pin, &p->s1[0], &typ4.f); 
//printf("VALUEf: fmt: \"%s\" %0.8f %0X\n",&p->s1[0],typ4.f,typ4.ui);
			pspec->slot[pspec->slotidx].x = typ4.ui; 	return 1;
		break;
	case  CALTYPE_DOUBLE:	sscanf(pin, &p->s1[0], &typ5.d);
//printf("VALUEd: fmt: \"%s\" %0.15f\n",&p->s1[0], typ5.d);
		pspec->slot[pspec->slotidx].x = typ5.ui[0]; 
			pspec->slot[pspec->slotidx+1].x = typ5.ui[1]; 	return 2;
		break;
	default:
		printf("line: %d, ERR: readcalvalue, numbertype: %d\n",linenumber, numbertype);
		break;
	}
	return 0;
}	
/******************************************************************************
 * static int checkrange_canid(int a);
 * static int checkrange_skip_unit(int a);
 * static int checkrange_skip_calib(int a);
 * static int checkrange_force(int a);
 * @brief 	: Check range and printf errors
 * @param	: a = value to be checked
 * return	: 0 = OK; -1 canid, -2 skip_unit, -3 skip_calib, -4 force
*******************************************************************************/
static int checkrange_canid(int a)
{
//	if ((a & CAN_DATATYPE_MASK) != 0) // 
//		{printf("line: %d ERR CAN ID: Upper bits of CAN ID must be zero: %08X\n",linenumber,(unsigned int)a); return -1;}
	if ((a & 0x1) != 0)
		{printf("line: %d ERR CAN ID: bit 0 must be zero: %08X\n",linenumber,(unsigned int)a); return -1;}
	if (((a & 0x4) == 0) && ((a & 0x001ffff8) != 0) )
		{printf("line: %d ERR CAN ID: 11 bit address with extended bit in ext field: CAN ID: %08X masked: %08X\n",linenumber,(unsigned int)a, (unsigned int)(a & 0x001ffff8)); return -1;}
	return 0;
}
static int checkrange_skip_unit(int a)
{
	if ((a < 0) || (a > 1))
		{printf("line: %d ERR skip_unit out of range: 0 = use; 1 = skip %d\n",linenumber,a); return -2;}
	return 0;
}
static int checkrange_skip_calib(int a)
{
	if ((a < 0) || (a > 2))
		{printf("line: %d ERR skip_calib: out of range: %d\n\t(1 = load with .txt data if crc not same) [unit == .txt]\n(1 = load with .txt data if crc not same) [unit == .txt]\n(2 = skip loading .txt data if unit calib crc is OK) [unit != .txt]\n",linenumber,a); return -3;}
	return 0;
}
static int checkrange_force(int a)
{
	if ((a < 0) || (a > 1))
		{printf("line: %d ERR force: out of range:  %d\n0 = crc-32 check skips;\n 1 = crc-32 check bypassed and all data sent to unit\n",linenumber,a); return -4;}
	return 0;
}

/******************************************************************************
 * static int canidcheck(struct CANIDETAL pidlist[], int idx);
 * @brief 	: Make list of can id's and check for duplicates
 * @param	: pidlist= pointer to struct array with CAN ID info
 * @param	: idx = index to most recently entered CAN ID.
 * @return	: 0 = OK; -1 = duplicate
*******************************************************************************/
static void printcanidcheck(struct CANIDETAL pidlist[], int idx, int i)
{
	printf("%2u 0x%08X %s @%s\n\ton line %d with\n", i, pidlist[idx].canid,pidlist[idx].name,pidlist[idx].description,pidlist[idx].linenumber+1);

	printf("   0x%08X %s @%s\n\t on line %d\n",pidlist[i].canid,pidlist[i].name,pidlist[idx].description,pidlist[i].linenumber+1);
}
static int canidcheck(struct CANIDETAL pidlist[], int idx)
{
	int i;
	for (i = 0; i < idx; i++)
	{
		/* Check binary CAN ID for duplicates. */
		if (pidlist[idx].canid == pidlist[i].canid)
		{printf("###ERR:     BINARY CAN ID duplicate: \n"); printcanidcheck(pidlist,idx, i); return -1;}

		/* Check for name duplicates. */
		if (strcmp(pidlist[idx].name, pidlist[i].name) == 0)
		{printf("###ERR: ASCII NAME CAN ID duplicate: \n"); printcanidcheck(pidlist,idx, i); return -2;}
	}
	return 0;
}
/******************************************************************************
 * void printcanidlist(struct CANIDETAL pidlist[], int idx);
 * @brief 	: Print list of can id info
 * @param	: pidlist= pointer to struct array with CAN ID info
 * @param	: idx = Number of entries in struct array
*******************************************************************************/
void printcanidlist(struct CANIDETAL pidlist[], int idx)
{
	int i;
	printf("CAN ID list, gleaned from input file\n");
	for (i = 0; i < idx; i++)
	{
		printf("%3d 0x%08X %s @%s\n",i, pidlist[i].canid,pidlist[i].name,pidlist[i].description);		
	}
	return;
}
/******************************************************************************
 * static void bombout(int n);
 * @brief 	: exit
 * @param	: n = Number to print
*******************************************************************************/
static void bombout(int n)
{
	printf("\nFIX and RERUN: err code %d\n", n);
	exit (n);
}

