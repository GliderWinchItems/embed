/******************************************************************************
* File Name          : parse.h
* Date First Issued  : 11/22/2014
* Board              : PC
* Description        : Read CAN .txt file and place into strut(s)
*******************************************************************************/

#ifndef __PARSETXT
#define __PARSETXT

#include "winch_merge2.h"	// struct CANCAL, plus some subroutines of use
#include "common_highflash.h"


#define CANIDDESCRIPTIONSIZE 96
#define CANIDNAMESIZE	48	// Size of CAN id name in #define
#define FORMATSIZE	48	// Max length of format field
#define NUMBEROFCALRAW	6	// Number of (//$ line) raw->display calibration per canid
#define CMDPLINESIZE	256	// Length of path/filename for .srec file
#define SUBSYSNAMESIZE	48	// Size of subsystem name field
#define NUMBEROFSUBSYS	6	// Number of subsystems allowable

// #define NUMBEROF4BYTSLOTS (see ..svn_discoveryf4/common_all/trunk/common_highflash.h)

/* #define name, can id and description */

// Note: in struct LDPROGSPEC the loader canid is separate from the app canids.
struct CANIDETAL // Can id and others
{
	unsigned int	linenumber;	// line number in .txt file for CAN ID 
	unsigned int	canid;		// binary CAN ID
	unsigned int	subsysindex;	// subsystem to which this CAN ID belongs
	unsigned int	refct;		// Number of references to this CAN ID
	char name[CANIDNAMESIZE];	// #define name
	char description[CANIDDESCRIPTIONSIZE]; // Description text (string)
	unsigned int	canslotidx;		// Current index into cancal[] (payload)
	struct	CANCAL	cancal[NUMBEROFCALRAW]; // Calibration of raw to display
};

struct SUBSYS
{
	int  relidx_slot;		// Relative index slot: 
	int  relidx_canid;		// Relative index canidetal
	int  number;			// Subsystem number
	char name[SUBSYSNAMESIZE];	// Subsystem name
	char structname[CANIDNAMESIZE];	// Name of struct used with this subsystem
	char description[CANIDDESCRIPTIONSIZE]; // Description text (string)
};

struct SLOTSTUFF
{
	unsigned int	linenumber;	// line number in .txt file for this slot
	u32	x;			// Value for slot (4 bytes binary)
	unsigned int	type;		// Type of value (e.g. float)
	char format[FORMATSIZE];	// Read in format (e.g. "%f")
	char description[CANIDDESCRIPTIONSIZE]; // Description text (string)
};

struct LDPROGSPEC // Holds path/file and CAN ID's extracted from input file
{
	char loadpathfile[CMDPLINESIZE]; // path/filename for .srec program
	struct CANIDETAL c_ldr;
	char	canid_name[CANIDNAMESIZE]; // CAN id name in #define for 'P'line
	unsigned int	force;		// 1 = force loading; skip crc-32 by-pass
	unsigned int	skip_unit;	// 1 = skip this spec; 0 = use
	unsigned int	skip_calib;	// 0 = skip FLASHP entirely

	unsigned int	idct;		// Current index into c_app, CAN IDs used in following array
	struct CANIDETAL c_app[NUMCANIDS];

	unsigned int 	slotidx;		// Current index into slot[], i.e. calibration, parameters, etc. loaded
	struct SLOTSTUFF slot[NUMBEROF4BYTSLOTS]; // Slots for app calibration, et al.

/*
NOTE: subsysnum is needed to handle the case where there are no subsystems on a unit, and therefore
the index will be zero, which is also the same index if there is just one subsystemn, which ties in
with the calcat update,...  Sort of messy, but it gets into the problem of having one list for 
calibrations, parameters, used CAN IDs, and noting where the each subsystem starts within the list.
*/
	unsigned int	subsysnum;	// Count of subsystems in this unit
	unsigned int	subsysct;	// Current index into subsystem
	struct SUBSYS subsys[NUMBEROFSUBSYS];
};

/******************************************************************************/
int parse(FILE* fpList, struct LDPROGSPEC pldp[], int size, struct CANIDETAL pidlist[], int idsize, int sw);
/* @brief 	: Load structs with data from .txt file
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
int parse_buildcanidlist(FILE* fpList, struct CANIDETAL pidlist[], int idsize);
/* @brief 	: Load a struct array with CAN IDs from .txt file
 * @param	: fpList = pointer to file
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
 * @return	: 0 = nothing to do; 
 * 		: + = Success--Number of struct items loaded into the struct array
 *		: -1 = error.
 *		: -2 = error, again.
*******************************************************************************/
 void printcanidlist(struct CANIDETAL pidlist[], int idx);
/*@brief 	: Print list of can id info
 * @param	: pidList= pointer to struct array with CAN ID info
 * @param	: idx = Number of entries in struct array
*******************************************************************************/
void parse_list_by_ID(struct CANIDETAL pidlist[], int idsize);
/* @brief 	: List CAN ID table by CAN ID (hex)
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/

#endif
