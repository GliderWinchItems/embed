/******************************************************************************
* File Name          : cmd_p.c
* Date First Issued  : 10/09/2014
* Board              : PC
* Description        : Program load for one unit
*******************************************************************************/
/*
./mm && ./cangate /dev/ttyUSB2 ../test/testmsg2B.txt
cd ~/svn_discoveryf4/PC/sensor/cangate/trunk
./mm && ./cangate 127.0.0.1 32123 ../test/testmsg2B.txt

Terms:
block	- one flash block of the stm32, e.g. 1024 or 2048 bytes
segment	- contiguous memory addresses for binary bytes in image built from .srec


10-11/2014 rev 246 working w/o highflash block
*/

#include "cmd_p.h"

#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include <string.h>
#include <zlib.h>
#include "common_fixedaddress.h"
#include "common_highflash.h"

#include "parse.h"
#include "timer_thread.h"

#define NOPRINTF	1	// parse.c: 0 = printf, not zero = don't printf

#define TIMER_RESPONSEWAIT	2000	// Number of milliseconds to wait for the unit to respond
#define WAITDELAY	(1024*5)	// Number of 2048 per sec ticks to squelch other units during loading



struct QUOTESTR
{
	int flag1;	// Return: length of s1; 0 = no data; 
	int flag2;	// Return: length of s2; 0 = no data;
	char s1[128];	// Extracted string between first pair of "
	char s2[128];	// Extracted string between second pair of "
};
struct QUOTESTR quotestr;

/* struct LDPROGSPEC holds the values extracted from the .txt file--
canid_app: APP CAN ID's come from lines in the .txt file with "#define ... // I" 
*/


struct SRECLN // .srec line
{
	int ct;				// Byte count
	u32 addr;			// Address
	unsigned int chk;		// Line checksum
	unsigned char byte[256];	// Data bytes & line checksum
};



struct PSEG // Pointers to contiguous segments of binary image
{ 	//	 Image segment
	unsigned int addr_start;// Beginning stm32 address
	unsigned int addr_end;	// (Ending + 1) stm32 address 
	unsigned char* pb;	// Point to beginning of binary image in this machine
	unsigned int chk;	// crc for this segment
	unsigned int skip;	// Skip crc check (send all)
	
	// Working pointers & counts for sending blocks.
	// PC image
	unsigned char* blkpc_pstart;	// PC Pointer to start of blk
	unsigned char* blkpc_pend;	// PC Pointer to end of blk + 1
	unsigned int   blkpc_bytct;	// PC byte count
	// STM32
	unsigned int blkstm32_pstart;	// STM32 Pointer to start of blk
	unsigned int blkstm32_pend;	// STM32 Pointer to end of blk + 1
	unsigned int blkstm32_base;	// STM32 addr of flash block start
	unsigned int blkstm32_bend;	// STM32 addr of flash block (end+1)
};
//static struct PSEG psegprog;	// Program segment loading (application program)
static struct PSEG psegflashh;	// High flash (FLASHH) loading (crc check & CAN ID)
static struct PSEG psegflashp;	// High flash (FLASHP) loading (calibration & parameters)

static int eos_flag; // End of segment flag
static int eob_flag; // End of block flag

static unsigned int flashp_addr;
static unsigned int flashp_size;


static struct HIGHFLASHH highflashh; // Image on this PC
//static u32 highflash_addr;	// STM32 address
static u32 fixedaddr;

//static struct HIGHFLASHP highflashp;	// Image on this PC
//static u32 highflashp_addr;	// STM32 address


/* Routine protoypes */
/* Routine protos */
static void cmd_p_do_msg(void);
unsigned int Crc32Fast(unsigned int* pData, unsigned int intcount);
static void flashblk(struct CANRCVBUF* pcan, struct PSEG* pseg);
static int setupandsend(struct CANRCVBUF* pcan, struct PSEG* pseg);

static void print_state(void);
static int dlc_check(struct CANRCVBUF* pcan, u32 dlc);
static void flashpsend_init(void);
static void flashhsend_init(void);
static int blk_pointers_first(struct PSEG* pseg);
static int blk_pointers_next(struct PSEG* pseg);


/* From cangate */
extern FILE *fpList;	// canldr.c File with paths and program names
extern int fdp;		// File descriptor for input file
extern FILE *fpList;
extern int fpListsw; 	// 0 = no test msg file list


/* List of program specs */
#define NUMPROGS	64	// Number of load specs
static struct LDPROGSPEC  ldp[NUMPROGS];
static int loadpathfileIdx = 0; // Index in program number
static int loadpathfileMax = 0; // Index+1 of last program number

/* List of CAN IDs extracted from input file. */
#define SIZECANID	512	// Max number array will hold
static struct CANIDETAL canidetal[SIZECANID];
static int idsize;	// Number stored in the foregoing array.

static int canseqnumber = 0;

/* State variables */
static 	int msg_p_sw  = 0;	// State for rcv msgs entry from cangate.c
static  int msg_p_sw1 = 10;	// Flash block-by-block loop state
        int cmd_p_sw  = 0;	// State for 'select' timeout entry from cangate.c
static	int cmd_p_sw_ret;	// cmd_p_sw return


/* List of contiguous segments */
#define SEGSIZE 1024
static struct PSEG seg[SEGSIZE];// Egads! we only expect one! (or two)
static int segIdx;		// Current segment index
static int segIdxmax;		// Number of contiguous segments in the .srec file

/* Get the flash block size from the STM32 */
static unsigned int flash_blk_size;	// Size of one flash block in the STM32

unsigned timeout_ctr = 0;	// Count entries from cangate timeout (1 ms).

static FILE* fpProg;		// .srec program file
static int first_sw = 0; 	// First load address of a segment
static unsigned char b[1048576];// Max program binary image size
static unsigned int bp[262144];	// FLASHP image can't be any bigger than this ( 1MB)
static time_t time_begin;


/* **************************************************************************************
 * u16 mv2(u8* p2);
 * @brief	: Convert 2 bytes into a 1/2 word
 * u32 mv4(u8* p2);
 * @brief	: Convert 4 bytes into a word

 * ************************************************************************************** */
u16 mv2(u8* p2){ return ( *(p2+1)<<8 | *p2 ); }
u32 mv4(u8* p2){ return ( *(p2+3)<<24 | *(p2+2)<<16 | *(p2+1)<<8 | *p2 ); }

/******************************************************************************
 * static void sendcanmsg(struct CANRCVBUF* pcan);
 * @brief 	: Send CAN msg
 * @param	: pcan = pointer to CANRCVBUF with mesg
*******************************************************************************/
static unsigned int RcvMsgCtr = 0;
static struct CANRCVBUF cansave;
static int retryctr = 0;
static int SendMsgCtr = 0;

static void sendcanmsg(struct CANRCVBUF* pcan)
{
	struct PCTOGATEWAY pctogateway; 
	pctogateway.mode_link = MODE_LINK;	// Set mode for routines that receive and send CAN msgs
	pctogateway.cmprs.seq = canseqnumber++;	// Add sequence number (for PC checking for missing msgs)
	USB_toPC_msg_mode(fdp, &pctogateway, pcan); 	// Send to file descriptor (e.g. serial port)
	cansave = *pcan;	// Save in case of a retry.
	timeout_ctr = 0;	// Reset timeout counter for response
SendMsgCtr += 1;
//printf("MSG # %d: %08x %d %08X %08X\n",SendMsgCtr, pcan->id, pcan->dlc, pcan->cd.ui[0],pcan->cd.ui[1]);
	
	return;
}
/******************************************************************************
 * static void sendCMD_simple(u8 cmd);
 * @brief 	: Send a command with only the command code payload byte
 * @param	: Command code (see: common_can.h)
*******************************************************************************/
static void sendCMD_simple(u8 cmd)
{
	struct CANRCVBUF can;
//	can.id = ldp[loadpathfileIdx].c_ldr.canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.id = ldp[loadpathfileIdx].c_ldr.canid; // Basic CAN ID is also CMD
	can.dlc = 1;		// Payload size
	can.cd.ull = 0; // JIC for less debugging confusion
	can.cd.uc[0] = cmd; 	// Add command code
	sendcanmsg(&can);	// Send msg 
	return;	
}
/******************************************************************************
 * static void sendCMD_25(u8 cmd, u32 n);
 * @brief 	: Send a command with command code payload byte + a 4 byte number
 * @param	: Command code (see: common_can.h)
 * @param	: 4 bytes that goes into payload bytes 2-5, (little endian)
*******************************************************************************/
static void sendCMD_25(u8 cmd, u32 n)
{
	struct CANRCVBUF can;
//	can.id = ldp[loadpathfileIdx].c_ldr.canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.id = ldp[loadpathfileIdx].c_ldr.canid; // Basic CAN ID is also CMD
	can.dlc = 5;		// Payload size
	can.cd.ull = 0; // JIC for less debugging confusion
	can.cd.uc[0] = cmd; 	// Command code
	can.cd.uc[1] = n; 	// Load 'n' (non-aligned)
	can.cd.uc[2] = n >> 8; 
	can.cd.uc[3] = n >> 16; 
	can.cd.uc[4] = n >> 24;
	sendcanmsg(&can);
	return;	
}
/******************************************************************************
 * static void sendCMD_crc32(unsigned char* pstart, unsigned char* pend);
 * @brief 	: Send CAN msg to app to request unit to reply with crc32
 * @param	: pstart = pointer to first byte in stm32 memory
 * @param	: pend   = pointer to last byte+1 in stm32 memory
*******************************************************************************/
static void sendCMD_crc32(unsigned int pstart, unsigned int pend)
{
	struct CANRCVBUF can;
//	can.id = ldp[loadpathfileIdx].c_ldr.canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.id = ldp[loadpathfileIdx].c_ldr.canid; // Basic CAN ID is also CMD
	can.dlc = 8;
	/* Size goes in uc[1] - uc[3] */
	can.cd.ui[0] = ((pend - pstart) << 8);	// Count of bytes [1] - [3]
	can.cd.ui[1] = pstart; // Start address [4] - [7]
	can.cd.uc[0] = LDR_CRC;
	sendcanmsg(&can);
printf("SEND CRC REQ: %08X %08X %08X\n",pstart, pend, pend-pstart);
	return;	
}

/******************************************************************************
 * static void sendRD(u32 count);
 * @brief 	: Send a READ data payload
 * @param	: Number of bytes to read-- 1-8
*******************************************************************************/
//static void sendRD(u32 count)
void sendRD(u32 count) // Avoid compiler warning
{
//	pcan->id &= 0x0ffffffe; 
	struct CANRCVBUF can;
	can.id = ldp[loadpathfileIdx].c_ldr.canid | (CAN_EXTRID_DATA_RD << CAN_DATAID_SHIFT);
	if (count > 8) count = 8;
	can.dlc = count;
	sendcanmsg(&can);
	return;	
}
/******************************************************************************
 * static void sendMASTERRESET_squelch(u32 waitct);
 * @brief 	: Send a command to stop the other units from sending CAN msgs
*******************************************************************************/
static void sendMASTERRESET_squelch(u32 waitct)
{
	struct CANRCVBUF can;
	can.id = CAN_SQUELCH; 	// High priority msg
	can.dlc = 8;		// Payload size
	can.cd.ui[0] = waitct; 	// Tick count for units to squelch sending
	// CAN ID in payload causes other units to stop sending
//	can.cd.ui[1] = ldp[loadpathfileIdx].c_ldr.canid | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);
	can.cd.ui[1] = ldp[loadpathfileIdx].c_ldr.canid; // Basic CAN ID is also CMD
	sendcanmsg(&can);	// Send msg 
printf("SQL # %d: %08x %d %d %08X\n",SendMsgCtr, can.id, can.dlc, can.cd.ui[0],can.cd.ui[1]);
	return;	
}
/******************************************************************************
 * int cmd_p_init(char* p);
 * @brief 	: Get path/file from input file for program loading
 * @param	: fpList = file with path/file lines
 * @param	: 0 = init was OK; -1 = failed
*******************************************************************************/
/* Originally this was much bigger */
int cmd_p_init(char* p)
{
	int ret;

	/* Build a list of CAN IDs */
	idsize = parse_buildcanidlist(fpList, canidetal, SIZECANID);
	if (idsize < 0){ cmd_p_sw = 0; printf("EXIT: FILE CAN ID EXTRACTION ERROR: %d\n", idsize);return -1; }

	/* Get the list of path/file for the CAN bus unit programs */
	ret = parse(fpList, &ldp[0], NUMPROGS, canidetal,idsize, NOPRINTF);	// Read (or re-read) input file and build list	
	if ( ret < 0 ) { cmd_p_sw = 0; printf("EXIT: FILE PARSE ERROR\n");return -1; }

	/* Re-initialize static variables */
	loadpathfileMax = ret;
	loadpathfileIdx = 0;
	RcvMsgCtr = 0;
	retryctr = 0;
	SendMsgCtr = 0;
	first_sw = 0; 		// First load address of a segment
	msg_p_sw  = 0;	// State for rcv msgs entry from cangate.c
	msg_p_sw1 = 10;	// Flash block-by-block loop state
	timeout_ctr = 0;	// Count entries from cangate timeout (1 ms).

	cmd_p_sw = 1; // Require re-initialization of program loading command p

	timer_thread_init(&cmd_p_do_msg,1000);	// Set callback for timer, and usec timer timeout count

	return 0;
}

/******************************************************************************
 * helper routine
*******************************************************************************/
static unsigned char hextobin(char *p)
{
	if (*p >= 'a') return (*p - 'a' + 10);
	if (*p >= 'A') return (*p - 'A' + 10);
	if (*p >= '0') return (*p - '0' +  0);
	return -1;
}
/******************************************************************************
 * helper routine
*******************************************************************************/
static unsigned char hextobin2(char *p)
{
	unsigned char b;
	b = hextobin(p) << 4;
	b |= hextobin( (p+1) );
	return b;
}
/******************************************************************************
 * convert srec line to binary image
*******************************************************************************/
static unsigned int convertline(struct SRECLN* pr, char *p)
{
	char* pp = p;  		// save for printf
	unsigned int uctmp = 0;	// Data-only byte summation
	unsigned char* pd = &pr->byte[0];
	int i;

	p += 2; // Skip 'Sx'
	pr->ct = hextobin2(p);
	p += 2;
	pr->chk = pr->ct+1;
	for (i = 0; i < (pr->ct); i++)
	{
		*pd = hextobin2(p); // Convert pair of input chars to binary
		pr->chk += *pd;	// Build checksum
		p += 2;		// Input pointer
		if (i > 3) uctmp += *pd;	// Build summation
		pd += 1;	// Output pointer
	}
	uctmp -= *(pd-1);	// Remove last byte (line checksum) from summation

//uctmp = 0;
//for(i = 4; i < pr->ct-1; i++)
// uctmp += pr->byte[i];

	/* Extract address */
	pr->addr = (pr->byte[0] << 24) | (pr->byte[1] << 16) | (pr->byte[2] << 8) | pr->byte[3];

//printf("ct: %d addr: %08X\n",pr->ct, pr->addr);
//for (i = 0; i < pr->ct; i++)
//	printf("%02x ",pr->byte[i]);
//printf("\n");

	/* Check received versus computed checksum for this srec line */
	if ( (pr->chk & 0xff) != 0 ) 
	{
		printf("Checksum error: %02x converted, %02x computed %s\n", (pr->byte[pr->ct-1]), (pr->chk & 0xff), pp);
		return 0;
	}

	return uctmp;
}

/******************************************************************************
 * helper routine
*******************************************************************************/
void seg_init(struct PSEG* pseg, unsigned char* pb)
{
	pseg->pb = pb;		// Save start of this segment
	pseg->chk = 0;		// Init checksum for this segment
	pseg->addr_start = -1;	// Show address not set
	pseg->addr_end = 1;
	return;
}
/******************************************************************************
 * static void cmd_p_do_msg(void);
 * @brief 	: Do program loading sequence: enters from timer (timer_thread.c)
 * @param	: fpList = file with test msgs
 * @param	: fd = file descriptor for serial port
*******************************************************************************/
static void cmd_p_do_msg(void)
{
	char cmd_p_buf[CMDPLINESIZE];	// File input buffer
	struct SRECLN srecln;	
//	int ret;
	unsigned int tmp;
	int linenumber;
	unsigned char* pb;	// Pointer to beginning of binary image segment

	switch (cmd_p_sw)
	{
	case 0:	// Command p not active state.
		return;

					
	case 1: // Start here when 'p' command given
		// Here, path/file and other lines of interest have been extracted from input file

	case 8: 
SendMsgCtr = 0; // Debugging counter: msgs sent
RcvMsgCtr = 0;  // Debugging counter: msgs received

		/* Skip this unit? */
		if (ldp[loadpathfileIdx].skip_unit != 0)
		{
			printf("#### SKIP Spec # %02d: %08X %s\n", loadpathfileIdx+1, ldp[loadpathfileIdx].c_ldr.canid, &ldp[loadpathfileIdx].loadpathfile[0]);
			cmd_p_sw = 82;	// Step to next spec
			return;
		}

		/* Open srec file */
		if ( (fpProg = fopen (&ldp[loadpathfileIdx].loadpathfile[0],"r")) == NULL)
		{ // Here, something wrong with path/file in the list.
			print_state();printf("$$$$$$$ ERROR--Prog file did not open: \nfile : %s\nindex: %d",&ldp[loadpathfileIdx].loadpathfile[0],loadpathfileIdx);
			perror("$$$$$$$$$  Reason for error");	// Reason for error
			cmd_p_sw = 0;	// Quit!
			return;

			/* Get next program, i.e. .srec file spec */
			loadpathfileIdx += 1;	// File spec index
			if (loadpathfileIdx >= loadpathfileMax)
			{ // Here, end of list
				print_state();printf("\n\nEND OF LIST OF .srec FILES: %d\n", loadpathfileMax);
				cmd_p_sw = 17;	// Set back to idle condition.
				cmd_p_sw_ret = 17;
				return;
			}
			else
			{
				cmd_p_sw = 8;	// Try again (1 ms later) with this new path/file
				return;
			}
		}
	
	/* Here, srec file opened.  Initialize for the first segment */
		/* Initialize for first segment of program file */
printf("\n");
printf("##################################################################################################\n");
printf("#### LOAD Spec # %d file: %s  loader canid: %08X\n", loadpathfileIdx+1, &ldp[loadpathfileIdx].loadpathfile[0],ldp[loadpathfileIdx].c_ldr.canid);
printf("##################################################################################################\n");
		
		pb = &b[0];		// Pointer to beginning of image storage	
		segIdx = 0;		// First segment index
		linenumber = 0;		// Count srec lines (mostly for debugging)
		first_sw = 0;		// First segment switch
		seg_init(seg, pb);	// Initialize segment descriptors
		time_begin = time(0);	// Save start time (secs) for timing duration.
 		
	/* Read .srec file and build binary image and pointers to contiguous segments */
		while ( (fgets (&cmd_p_buf[0],CMDPLINESIZE, fpProg)) != NULL)	// Get a line
		{ // Here not end of file
			linenumber += 1;	// Count line numbers
			if (cmd_p_buf[0] != 'S')
			{ // First char of line of a srec file should always be 'S'
				print_state();printf("S-REC ERROR: ====> 1st char not 'S' at line number: %d <====\n", linenumber);
			}
			else
			{ // Deal with remainder of S rec line
				tmp = convertline(&srecln,cmd_p_buf);	// Convert ascii to hex etc.
//printf("srec type: %c line number: %d\n",cmd_p_buf[1], linenumber);

				switch (cmd_p_buf[1])	// Type of S rec
				{
				case '0': // ID line
					cmd_p_buf[ (2*(srecln.ct - 2))] = 0; srecln.byte[(strlen(cmd_p_buf)/2 + 1 )] = 0; 
					printf("S0: %s\n",&srecln.byte[2]);
					break; // Get next line

				case '3': // 32b data line
					// Copy srec binary data to image array
					if (seg[segIdx].addr_end != srecln.addr) // Is this a new segment?
					{ // Here there is a discontinuity of the addresses, so it is a new segment
//printf("1addr_start: %08X ",seg[segIdx].addr_start);
//printf("addr_end: %08X new addr: %08X\n", seg[segIdx].addr_end, srecln.addr);
						if (first_sw == 0)
							first_sw = 1;
						else
							segIdx += 1; // Advance to next seg descriptor

						if (segIdx >= SEGSIZE)
						{
							print_state();printf("ERR: BAD--TOO MANY SEGMENTS!  We must quit! At srec line number: %d in path/file: %s\n",linenumber, &ldp[loadpathfileIdx].loadpathfile[0]);
							cmd_p_sw = 81; // Try again (2 ms later) with a new path/file
							return;
						}
						seg_init(seg, pb); // Initialize new segment descriptor.
						seg[segIdx].addr_start = srecln.addr;
						seg[segIdx].addr_end = srecln.addr; // Start with end = start
					}


					/* Copy data from converted srec to binary image array */
//printf("%d %d %s",linenumber,srecln.ct, cmd_p_buf);
					memmove(pb, &srecln.byte[4], srecln.ct-5); // Copy data from line to image
					pb += srecln.ct-5;			// Advance pointer in image array
					seg[segIdx].addr_end += srecln.ct-5;	// Update end address
					seg[segIdx].chk += tmp;			// Add data byte summation to segment checksum
					break; // Get next line

				case '7':	// Ending line with start address
					if (srecln.addr != *(u32*)&b[4]) // Compare jump addresses
					{ // Here, vector doesn't match S7 address
						print_state();printf("Note: srec jmp address (%08X) doesn't match vector (%08X)\n",srecln.addr, *(u32*)&b[4] );
					}
					else
					{ // Here, all is well.  Sooth the hapless Op with messages of goodness.
					print_state();
						printf("SUCCESS: srec jmp addresses match: (%08X) == (%08X)\n",srecln.addr, *(u32*)&b[4] );
						printf("addr_end:   %08X\n image idx: %08X %i \n", (unsigned int)seg[segIdx].addr_end, (unsigned int)(pb - &b[0]), (unsigned int)(pb - &b[0]));
						printf("addr_start: %08X\n",seg[segIdx].addr_start);
						printf("b[] idx:    %08X\n",(unsigned int)(&b[0] - seg[segIdx].pb));
						cmd_p_sw = 4; // SUCCESS: BINARY IMAGE(S) HAVE BEEN BUILT	

					}
					break; // Get next line (should be EOF)

				default: // What!
					print_state();
					printf("default srec type: %c  ",cmd_p_buf[1]);
					printf("S-rec type not 0, 3, or 7! at line number: %d %s\n", linenumber, &cmd_p_buf[0]);
				}
			}
		}
		segIdxmax = segIdx + 1;	// Number of contiguous segments in the .srec file
		segIdx = 0;		// Start with first segment.
		cmd_p_sw = 4;

	case 4: // Beginning CAN bus & STM32 work

printf("DONE with building image from srec file.  Number of segments in this srec file: %d\n", segIdxmax);
//cmd_p_sw = 0; // Set to idle
//int i;
//int tadd = 0x8007580;
//int torg = 0x8004000;
//for (i = 0; i < 512; i++){
// if ((i % 16) == 0) printf("%08X: ",(tadd+i));
// printf("%02X ",b[(tadd-torg + i)]);
// if ((i % 16) == 15) printf("\n");
// }
//printf("\n");

unsigned char* px = seg[segIdx].pb;
unsigned int sizey = (seg[segIdx].addr_end - seg[segIdx].addr_start);
unsigned char* pe = px + sizey;
unsigned int ck = 0;
printf("Idx: %d Size: %u\n",segIdx,sizey);
while(px < pe) ck += *px++;
printf ("Segment simple summation type checksum check: computed: %u segment: %u\n",ck,seg[segIdx].chk);

		/* ========== This starts the send wait-for-response sequence ================ */
		/* Send a 1 byte RESET command to this unit.  Then wait for it to respond. */
		sendCMD_simple(LDR_RESET); // Send a 1 byte payload command code

		// Use zlib routine to compute crc-32
		seg[segIdx].chk = crc32(0, (unsigned char*)seg[segIdx].pb, (seg[segIdx].addr_end - seg[segIdx].addr_start));

		if (ldp[loadpathfileIdx].force == 0) // "Force" logically means don't check crc-32's
			printf("Segment CRC-32: %08X\n",seg[segIdx].chk);
	
		cmd_p_sw_ret = 6;	// Handle high flash next
		cmd_p_sw = 5;		// Wait for time out. CAN msgs arrive in the 'cmd_p_do_msg1' below.
		msg_p_sw = 0;		// 'switch' in 'cmd_p_do_msg1' for first arriving CAN msg.
		break;

	case 5: /* Time waiting for a response from the unit.		
		Note: CAN msg responses arrive in calls 'cangate.c' makes to  'cmd_p_do_msg1' below.
		- 'sendCMD_simple(LDR_RESET);' above starts the send/response.  
		- The sequence continues in 'cmd_p_do_msg1'.
		- 'timeout_ctr' is reset each time a CAN msg is sent.
		*/
		timeout_ctr += 1;	// Count ticks (1 ms, see: cangate.c)
		if (timeout_ctr >= TIMER_RESPONSEWAIT)
		{ // Here we timed out waiting for a response from the unit
			print_state();printf("TIMEOUT: retry: %d\n", retryctr);
			retryctr += 1;	if (retryctr < 4)
			{
				sendcanmsg(&cansave); break;
			}
			retryctr = 0;
			cmd_p_sw = 81;	// Get next .srec file
			msg_p_sw = 0;
		}
		break;

	case 6: // End of program segments: handle high flash regions

		/* ############################# Build image of FLASHH ################################################ */
print_state();printf("SETUP PROGRAM CRC CHECK and CAN ID's in FLASHH memory region\n");
		/* Here-- end of segment: Build image of struct elements used to crc-32 check the program. */

		// "WTF!" (Write The 0xF, (since flash is erased to 0xFs)).
		int i;
		for (i = 0; i < NUMCANIDS; i++) highflashh.appcanid.canid[i] = ~0;	// Unused CAN ID's to all f's
		for (i = 0; i < NUMPROGSEGS; i++) 			// Unused Prog segments to all f's
			{highflashh.crcprog.crcblk[i].crc = ~0; highflashh.crcprog.crcblk[i].pstart = ~0; highflashh.crcprog.crcblk[i].count = ~0;}

		// Set count of number of program segments (jic check for overrunning array)
		highflashh.crcprog.numprogsegs = segIdxmax; 
		if (segIdxmax >= NUMPROGSEGS) highflashh.crcprog.numprogsegs = NUMPROGSEGS-1;

		// Set addresses and checksums for each segment
		for (i = 0; i < highflashh.crcprog.numprogsegs; i++)
		{
			highflashh.crcprog.crcblk[i].pstart =  seg[i].addr_start;	// Address
			highflashh.crcprog.crcblk[i].crc    =  seg[i].chk;		// crc-32
			highflashh.crcprog.crcblk[i].count  = (seg[i].addr_end - seg[i].addr_start); // count
		}

		// Set count of number of App CAN ID's
		highflashh.appcanid.numcanids = ldp[loadpathfileIdx].idct;		// Number of App CAN ids
		if (highflashh.appcanid.numcanids >= NUMCANIDS) highflashh.appcanid.numcanids = NUMCANIDS-1; // jic

		// Set App CAN ID's
		for (i = 0; i < ldp[loadpathfileIdx].idct; i++)	// Copy App CAN ids into highflashlayout struct
			highflashh.appcanid.canid[i] = ldp[loadpathfileIdx].c_app[i].canid;

/* NOTE: These are hard-coded versions numbers...done for expedience.  */
		// Place a version number 
		highflashh.crcprog.version = 1;

		// Place a version number 
		highflashh.appcanid.version = 1;

		// Place a version number 
		highflashh.version = 1;

		// Compute crc-32 on struct CRCPROG. */
		highflashh.crcprog.crc = crc32(0, (unsigned char*)&highflashh.crcprog.version, (sizeof(struct CRCPROG) - sizeof(u32) ) );
	
		// Compute crc-32 on struct except crc at the beginning
		highflashh.appcanid.crc = crc32(0, (unsigned char*)&highflashh.appcanid.version, (sizeof(struct APPCANID) - sizeof(u32) ) );

		// Compute crc-32 on whole FLASHH except crc at the beginning 
		highflashh.crc = crc32(0, (unsigned char*)&highflashh.version, (sizeof(struct HIGHFLASHH) - sizeof(u32) ) );
//printf("FLASHH crc: %08X sizeof: %08X\n",highflashh.crc, (unsigned int)(sizeof(struct HIGHFLASHH) - sizeof(u32)) );
//for (i = 0; i < 0x80; i++)
//{
//	printf("\t%2u 0x%08X\n",(unsigned int)highflashh.appcanid.canid[i]);
//}

		/* ========== This starts the send, wait for response sequence for flashh ================ */
		// Start sequence for sending the high flash struct.  
		print_state();printf("START HIGH FLASH; SEND CMD \"LDR_HIGHFLASHH\": get __highflashlayout address\n");
			
			sendCMD_simple(LDR_HIGHFLASHH);	// Request stm32 address of fixed loader struct
			msg_p_sw = 20;		// Next step when/if msg is received
			cmd_p_sw_ret = 61;	// Get next path/file after image is complete
			cmd_p_sw = 5;		// In the meantime check for time out
		break;

	case 61: // End of FLASHH loading.
		if (ldp[loadpathfileIdx].skip_calib == 0)	// Check option: 0, 1, 2 for calibration update/loading
		{ // Here.  Skip the FLASHP procedure entirely.
print_state();printf("SKIPPING FLASHP entirely--skip_calib: %i\n", ldp[loadpathfileIdx].skip_calib);
			cmd_p_sw = 81; // Upon next "tick" get next srec/file spec
			break;
		}
		if (ldp[loadpathfileIdx].slotidx < 1)	// Check option: 0, 1, 2 for calibration update/loading
		{ // Here, no calibrations to load so skip
print_state();printf("SKIPPING FLASHP no calibrations in input file: skip_calib: %i\n", ldp[loadpathfileIdx].skip_calib);
			cmd_p_sw = 81; // Upon next "tick" get next srec/file spec
			break;		
		}

print_state();printf("LOADING FLASHP skip_calib code: %i number of calibrations: %i\n", ldp[loadpathfileIdx].skip_calib, ldp[loadpathfileIdx].slotidx);

		/* ############################### Build image of FLASHP ################################################### */
print_state();printf("SETUP or CHECK FLASHP memory region\n");
		// We'll use the byte array that held the program image since it is 1 MB

// hard coded test to see that ascii fields got set correctly
//printf("\n\tASCII FLASHP: ");
//for (i = 6; i < 6+7; i++)
//  printf("%c%c%c%c", ldp[loadpathfileIdx].slot[i].x,ldp[loadpathfileIdx].slot[i].x>>8,ldp[loadpathfileIdx].slot[i].x>>16,ldp[loadpathfileIdx].slot[i].x>>24);

		for (i = 0; i < ldp[loadpathfileIdx].slotidx; i++)
		{ // Skip crc slot, and add 'int's with calibrations and parameters
			bp[i]    = ldp[loadpathfileIdx].slot[i].x;	//
		}
//printf("\n BP: %s\n",(char*)&bp[7]);

		// Compute crc-32 on struct except crc at the beginning
		bp[0] = crc32(0, (unsigned char*)&bp[1], (ldp[loadpathfileIdx].slotidx * sizeof(u32)) );
print_state();printf("bp: crc: %08X",bp[0]);
for (i = 1; i < 6; i++) printf(" %08X",bp[i]);
printf("\n");

		/* ========== This starts the send, wait for response sequence for flashp ================ */
		// Start sequence for sending the  FLASHP
		sendCMD_simple(LDR_HIGHFLASHP); // Request size and address for FLASHP app struct
		cmd_p_sw_ret = 81;	// Get next srec/file spec after sending
		cmd_p_sw     = 5;	// In the meantime check for timeout
		msg_p_sw1    = 10;	// 
		msg_p_sw     = 40;	// Next step when/if msg is received		

		break;

	case 80: // Get next program segment
		
		segIdx += 1;	// Segment index
		if (segIdx < segIdxmax)
		{	
printf("NEXT SEGMENT\n");
		seg[segIdx].blkpc_pend    = seg[segIdx].pb;			// Pointer into pc image start for this segment	
		seg[segIdx].blkstm32_pend = seg[segIdx].addr_start;		// "Pointer" (u32 number) to stm32 start address 
		sendCMD_crc32  (seg[segIdx].addr_start, seg[segIdx].addr_end);	// Request crc-32 from unit
		cmd_p_sw_ret = 80;	// Get next segment after segment is been sent
		cmd_p_sw     = 5;	// In the meantime check for timeout
		msg_p_sw1    = 10;	// 
		msg_p_sw     = 40;	// Next step when/if msg is received		
		break; 
		}
		
	case 81: // Get next path/file name spec
		print_state();printf("END OF SEGMENTS: get next .srec FILE: %d\n", loadpathfileMax);
		sendCMD_simple(LDR_RESET); // Reset and let loader jump to app
			
		printf("TIME DURATION: %u (secs)\n", (u32)(time(0) - time_begin));

	case 82:
		/* Get next program, i.e. .srec file spec */
		loadpathfileIdx += 1;	// File spec index

		if (loadpathfileIdx >= loadpathfileMax)
		{ // Here, end of list
print_state();printf("\n\nEND OF LIST OF .srec FILES: %d\n", loadpathfileMax);
			cmd_p_sw = 17;	// Set back to idle condition.
			cmd_p_sw_ret = 17;
		}
		else
		{ // Here open next .srec file in the list
print_state();printf("NEXT .srec FILE: %u out of %u\n", loadpathfileIdx, loadpathfileMax);
				cmd_p_sw = 8;
		}
		break;

	case 17:
		break;
	
	default:
	print_state();printf("DEFAULT: cmd_p_sw %d\n", cmd_p_sw);	
	}
	return;
}
/******************************************************************************
 * int checkCMDmsg(struct CANRCVBUF* pcan, u8 cmd, int dlc);
 * @brief 	: Check that CAN msg has the expected ID and payload
 * @param	: pcan = pointer to struct with CAN msg
 * @param	: cmd = command code expected in first payload byte
 * @param	: dlc = payload size expected
 * return	: 0 = passed all checks; not zero = failed
*******************************************************************************/
int checkCMDmsg(struct CANRCVBUF* pcan, u8 cmd, int dlc)
{
	/* Check that command code is expected code, and dlc is correct. */
	if ( (pcan->cd.uc[0] != cmd) || (dlc_check(pcan,dlc) != 0)  )
	{ // Here, it is not what we expected.  Is an unexpected RESET?
		if ( (pcan->cd.uc[0] != LDR_RESET)  )
		{ // Here, yes.  Go to the next path/file.  The target unit rebooted on us.
			cmd_p_sw = 81;
		}
		return -1;	// Return failed to meet our expec
	}
	// Here, the msg matches what we expect.
	retryctr = 0;	// Reset retry counter
	return 0;
}
/******************************************************************************
 * void cmd_p_do_msg1(struct CANRCVBUF* pcan);
 * @brief 	: Deal with incoming CAN msgs
 * @param	: pcan = pointer to struct with CAN msg
*******************************************************************************/
/* 
Note:  This routine is entered when there is a CAN msg.  When a msg arrives that
we expect, then the next step is taken.  The sequence is to send a CAN msg that will
result in a response.  When the response is received the next CAN msg is sent.  The 
'cmd_p_do_msg' routine above is entered when there are 'select' timeouts in the main
loop of 'cangate.c'.  These are 1 ms ticks.  When the count reaches a threshold the
loading sequence is aborted.
*/ 
void cmd_p_do_msg1(struct CANRCVBUF* pcan)
{
	/* Select msgs that are for the unit ID */
	// Mask can id with 0x0ffffffe (unless we change common_can.h)
//	if ( (pcan->id & ~(CAN_EXTRID_MASK | 1)) == ldp[loadpathfileIdx].c_ldr.canid)
	if ( (pcan->id & ~0x1) == ldp[loadpathfileIdx].c_ldr.canid)
	{ // Here, we have the base address for this unit

RcvMsgCtr += 1; // debug cound rcv msgs
//printf("RCV # %d: %08x %d %08X %08X\n",RcvMsgCtr, pcan->id, pcan->dlc, pcan->cd.ui[0],pcan->cd.ui[1]);

		switch (msg_p_sw)
		{
		case 0:	// Looking for RESET msg response
			// Check that CAN msg ID, command code, and payload count is what is expected.
			//             CAN msg  Command  dlc
			if (checkCMDmsg(pcan, LDR_RESET, 1) != 0) break; // Return not expected msg
			// Here, unit reset, booted up, and loader sent response which means "ready"
print_state();printf("GOT RESET: ");
			if (ldp[loadpathfileIdx].force == 0) // "Force" (i.e. send all data to unit)
			{ // Here.  Use crc check to determine if sending is necessary.
			// Here, unit reset, booted up, and loader sent response which means "ready"
printf("SEND CMD \"LDR_CRC\": crc-32 request %08X %08X %08X\n",seg[segIdx].addr_start, seg[segIdx].addr_end,seg[segIdx].addr_end - seg[segIdx].addr_start);
				sendCMD_crc32(seg[segIdx].addr_start, seg[segIdx].addr_end);	// Request crc32 from unit
				msg_p_sw = 1;		// Next step when/if msg is received
				break;
			}
			// Here, skipping crc check on total segment.
print_state();printf("SKIP CRC-32 CHECK SEGMENT.  SEND CMD \"LDR_FLASHSIZE\": get flash size\n");
			sendCMD_simple(LDR_FLASHSIZE); // Request flash block size
			msg_p_sw = 2;		// Next step when/if msg is received			
			break;

		case 1: // Looking for CHECKSUM from unit
			if (checkCMDmsg(pcan, LDR_CRC, 8) != 0) break;
print_state();printf("GOT CRC-32. unit: %08X pc: %08X \n", pcan->cd.ui[1], seg[segIdx].chk);
			// Here we got a CRC response.  See if ours and theirs match
			if (pcan->cd.ui[1] == seg[segIdx].chk)
			{ // Here, there was a match so we can skip loading this segment
printf("HUZZAH! CRC-32 SEGMENT MATCH: segment %d.  Get next segment.\n", segIdx);
				msg_p_sw = 0;
				cmd_p_sw = cmd_p_sw_ret;	// Get next segment	
				break;
			}

			// Looks like we need to load this program segment
print_state();printf("WAH WAH WAH CRC-32 MISMATCH: %08X PC's crc: %08X, ...so reload program\n", pcan->cd.ui[1], seg[segIdx].chk);
printf("\t\tLDR_CRC Reply: id: %08X dlc: %d ui[0]: %08X ui[1]: %08X\n", pcan->id,pcan->dlc,pcan->cd.ui[0],pcan->cd.ui[1]);
print_state();printf("SEND CMD \"LDR_FLASHSIZE\": get flash size\n");
			sendCMD_simple(LDR_FLASHSIZE); // Request flash block size
			msg_p_sw = 2;		// Next step when msg is received			
			break;

		case 2: // Looking for FLASH BLOCK SIZE from unit
			if (checkCMDmsg(pcan, LDR_FLASHSIZE, 3) != 0) break; // Return not expected msg

			// Here, we got the FLASH BLOCK SIZE
			flash_blk_size = (pcan->cd.uc[1] | (pcan->cd.uc[2] << 8) ); // Get flash size
print_state();printf("GOT \"LDR_FLASHSIZE\": %d \n",flash_blk_size);
			if (!((flash_blk_size == 1024) || (flash_blk_size == 2048))) 
				{print_state();printf("\t\t### ARGH!!! FLASH SIZE LOOKS WRONG: %d.  Get next segment.\n",flash_blk_size);
				msg_p_sw = 0;
				cmd_p_sw = cmd_p_sw_ret;	// Get next segment	
				break;
			}

			/* Initialize for going through segment sending CAN payloads to STM32 flash blocks. */
			msg_p_sw = 3;		// Next step when/if msg is received	
			msg_p_sw1 = 10;
			seg[segIdx].blkpc_pstart     = seg[segIdx].pb;			// Pointer into pc image start for this segment	
			seg[segIdx].blkstm32_pstart  = seg[segIdx].addr_start; 		// Pointer to corresponding stm32 start address 
			seg[segIdx].blkstm32_base    = seg[segIdx].addr_start & ~(flash_blk_size - 1); // Base address of STM32 flash block
			seg[segIdx].skip             = ldp[loadpathfileIdx].force;	// Skip crc-32 checks (which forces sending all)
			eos_flag = blk_pointers_first(&seg[segIdx]);			// First block working pointer setup

print_state();printf("\tSome data at the beginning of the segment image for checking against .list file\n");
int h; unsigned char* y = seg[segIdx].blkpc_pstart;for (h = 0; h < 32; h++)printf("%02x ",*y++); printf("\n");
			
		case 3: // Go down the blocks of flash and load any that don't have matching crc32's (unless "force" switch is ON)
			flashblk(pcan, &seg[segIdx]);	// Send the segment: setting the address and and sending all the payloads.  
			break;	// Keep coming back to this 'case' until 'flashblk' or something else below changes the 'cmd_p_sw'

		/* -------------- High flash transfer --------------------- */
		case 20: // Looking for High flash address
			if (checkCMDmsg(pcan, LDR_HIGHFLASHH, 5) != 0) break; // Return not expected msg

			sendMASTERRESET_squelch(WAITDELAY);	// Stop other units from sending CAN msgs.
			fixedaddr = (pcan->cd.uc[1] | (pcan->cd.uc[2] << 8) | (pcan->cd.uc[3] << 16)| (pcan->cd.uc[4] << 24));
print_state();printf("GOT highflashlayout: \"LDR_HIGHFLASHH\": get __highflashlayout address: %08X\n",fixedaddr);

			sendCMD_simple(LDR_FLASHSIZE); // Request flash block size
			msg_p_sw = 21;
			break;

		case 21: // Looking for FLASH BLOCK SIZE from unit
			if (checkCMDmsg(pcan, LDR_FLASHSIZE, 3) != 0) break; // Return not expected msg
			// Here, we got the FLASH BLOCK SIZE
			flash_blk_size = (pcan->cd.uc[1] | (pcan->cd.uc[2] << 8) ); // Get flash size
print_state();printf("GOT \"LDR_FLASHSIZE\": %d \n",flash_blk_size);
			if (!((flash_blk_size == 1024) || (flash_blk_size == 2048))) 
				{print_state();printf("\t\t&&&&&& ARGH! FLASH SIZE LOOKS WRONG: %d.  Get next segment.\n",flash_blk_size);
				msg_p_sw = 0;
				cmd_p_sw = cmd_p_sw_ret;	// Get next segment	
				break;
			}

			flashhsend_init();		// Setup struct with pointers etc.
print_state();printf("fixedaddr: %08X\n",fixedaddr);
print_state();printf("pstart: %08X pend: %08X\n", psegflashh.blkstm32_pstart, psegflashh.blkstm32_pend);
			eos_flag = blk_pointers_first(&psegflashh);	// First block working pointer setup


print_state();printf("SEND CMD \"LDR_CRC\": crc-32 request: pstart: %08X pend: %08X\n", (unsigned int)(fixedaddr + sizeof(u32) ), (unsigned int)(fixedaddr + (sizeof(struct HIGHFLASHH))) );
print_state();printf("pstart: %08X pend: %08X\n", psegflashh.blkstm32_pstart, psegflashh.blkstm32_pend);
			sendCMD_crc32( (fixedaddr + sizeof(u32) ), fixedaddr + (sizeof(struct HIGHFLASHH))  );	// Request crc32 from unit
msg_p_sw = 17; // <======================= debug halt here
			msg_p_sw = 23;
			break;

		case 23: // Looking for CRC
			if (checkCMDmsg(pcan, LDR_CRC, 8) != 0) break;
print_state();printf("GOT CRC-32. \n");
			// Here we got a CRC response.  See if ours and theirs match
			if (pcan->cd.ui[1] == highflashh.crc)
			{ // Here, there was a match so we can skip loading FLASHH
printf("HUZZAH! CRC-32 FLASHH MATCH.  Skip loading FLASHH. Get FLASHP address.\n");
				msg_p_sw = 17;
				cmd_p_sw = cmd_p_sw_ret;	// Get next
				break;	
			}

			// Looks like we need to load FLASHH
print_state();printf("WAH WAH WAH CRC-32 MISMATCH: %08X PC's crc: %08X, ...so reload FLASHH\n", pcan->cd.ui[1], highflashh.crc);
printf("\t\tLDR_CRC Reply: id: %08X dlc: %d ui[0]: %08X ui[1]: %08X\n", pcan->id,pcan->dlc,pcan->cd.ui[0],pcan->cd.ui[1]);

			/* Initialize for going through FLASHH sending CAN payloads to STM32 flash blocks. */
			msg_p_sw  = 24;			// Next step when/if msg is received
			msg_p_sw1 = 10;	
			flashblk(pcan, &psegflashh);	// Start sending of image			
			break;

		case 24: // Go down the blocks of flash and load any that don't have matching crc32's (unless "force" switch is ON)
			flashblk(pcan, &psegflashh);	// Continue sending struct until "somebody down there" sets 'cmd_p_sw'
			break;

		/* ------------------ FLASHP transfer --------------------- */
		case 40: // FLASHP
			if (checkCMDmsg(pcan, LDR_HIGHFLASHP, 8) != 0) break; // Return not expected msg
print_state();printf("GOT \"LDR_HIGHFLASHP\": addr: %08X size: %d \n", pcan->cd.ui[1],pcan->cd.ui[0] & 0x00ffffff);
			flashp_addr = pcan->cd.ui[1];
			flashp_size = (pcan->cd.ui[0] >> 8) & 0x000fffff; // Limit to 1 MB
			if (flashp_size == 0) 
				{print_state();printf("OOPS! 'flashp_size' unit returned zero\n"); cmd_p_sw = cmd_p_sw_ret; break;}


print_state();printf("SEND CMD \"LDR_CRC\": crc-32 request: pstart: %08X pend: %08X\n", (unsigned int)( flashp_addr + sizeof(u32) ), (unsigned int)(flashp_addr + flashp_size ) );
			sendCMD_crc32( ( flashp_addr + sizeof(u32) ), (flashp_addr + flashp_size ) );	// Request crc32 from unit
			msg_p_sw  = 41;
			msg_p_sw1 = 10;	
			break;

		case 41: // Looking for CRC-32 			
			if (checkCMDmsg(pcan, LDR_CRC, 8) != 0) break;
			// Here we got a CRC response.
			if (ldp[loadpathfileIdx].skip_calib == 1)	// Check option: we use "ours" as the source
			{ // Here, (option 1) we want the unit/application to have the same calib as ours
print_state();printf("GOT CRC-32 FLASHP. \"Our\" calib is source \n");
				psegflashp.chk = pcan->cd.ui[1] ;
				if (psegflashp.chk == bp[0]) // Compare crc-32s: theirs versus ours
				{ // Here, there was a match so we can skip loading FLASHP
printf("HUZZAH! CRC-32 FLASHP MATCH.  End loading unit.\n");
					cmd_p_sw = cmd_p_sw_ret;
					break;
				}
				else
				{ // Here, unit's crc doesn't match our image's crc.  Send our image to the unit
					/* Initialize for going through segment sending CAN payloads to STM32 flash blocks. */
printf("WAH WAH WAH CRC-32 MISMATCH: %08X PC's crc: %08X, ...so reload FLASHP\n", pcan->cd.ui[1], bp[0]);
					flashpsend_init();		// Setup struct with pointers etc.
					eos_flag = blk_pointers_first(&psegflashp);	// First block working pointer setup
					msg_p_sw = 43;
					msg_p_sw1 = 10;	
					flashblk(pcan, &psegflashp);	// Start sending of image
					break;
				}				
			}
			// Here, (option 2) the unit calib is king, however see if it is OK.
print_state();printf("GOT CRC-32 FLASHP. \"Their\" calib is source.  See if unit has a correct crc \n");
print_state();printf("SEND CMD \"LDR_RD4\": get FLASHP crc-32 that was in the unit\n");
			sendCMD_25(LDR_RD4, flashp_addr);	// Read 4 bytes at address provided in payload		
			msg_p_sw = 42;
			break;

		case 42: // Looking for 4 bytes returned
			if (checkCMDmsg(pcan, LDR_RD4, 5) != 0) break;
print_state();printf("GOT Stored CRC-32 FLASHP. ");
			if (pcan->cd.ui[1] == psegflashp.chk) // Compare crc-32s: their's stored versus their's computed
			{ // Here, there was a match so we can skip loading FLASHP and the unit's calib stands
printf("HUZZAH! CRC-32 FLASHP MATCH.  End loading FLASHP.\n");
				cmd_p_sw = cmd_p_sw_ret;
				break;
			}
			// Here, unit's crc doesn't match its computed crc.  Send our image to the unit as a default
			flashpsend_init();		// Setup struct with pointers etc.
			eos_flag = blk_pointers_first(&psegflashp);	// First block working pointer setup
			msg_p_sw1 = 10;
			msg_p_sw = 43;
			flashblk(pcan, &psegflashp);	// Start sending of image

			break;

		case 43: 
			flashblk(pcan, &psegflashp);	// Continue sending of image until somebody down there sets 'cmd_p_sw'
			break;

		case 17: // Idle
			if (checkCMDmsg(pcan, LDR_RESET, 1) != 0) 
			{
print_state();printf("IDLE: RCV'd msg: id: %08X dlc: %i ui[0]: %08X ui[1]: %08X\n",pcan->id, pcan->dlc, pcan->cd.ui[0], pcan->cd.ui[1]);				
				break;
			}
			// Here, we got a RESET unexpectedly.
			print_state();printf("WHAT!!! UNEXPECTED RESET: restart \n");		
			cmd_p_sw = 81;	// Get next path/file
			break;

		default:
			print_state(); printf("default: BOGUS SWITCH VALUE msg_p_sw: %d\n", msg_p_sw);
			break;
		}
	}
	return;
}
/******************************************************************************
 * static void flashblk(struct CANRCVBUF* pcan, struct PSEG* pseg);
 * @brief 	: Write an image in the PC to the stm32
 * @param	: pcan = pointer to CAN msg
*******************************************************************************/
static void flashblk(struct CANRCVBUF* pcan, struct PSEG* pseg)
{
	unsigned int crc;

	switch (msg_p_sw1)
	{
	case 9: // Looking for response that the STM32 has completed the flash write.
		if (checkCMDmsg(pcan, LDR_WRBLK, 3) != 0) break; // Return not expected msg
print_state();printf("GOT WRBLK: %08X\n", pseg->blkstm32_pstart);

		if ((pcan->cd.uc[1] != 0) || (pcan->cd.uc[2] != 0)) // Look at FLASH write error return bytes
		{ print_state();printf ("FLASH WRITE ERROR returned: write %x  erase %x\n", pcan->cd.uc[1], pcan->cd.uc[2]); }

		if (pseg->blkstm32_pstart >= pseg->addr_end)
		{ // Here we have reached the end of the segment/image
print_state();printf("CONGRATULATIONS! Segment/image complete\n");
			msg_p_sw = 17;
			cmd_p_sw = cmd_p_sw_ret; // Get next segment/image
			break;
		}

	case 10: // Initially entered w flash block size; next time after STM32 wrote flash.  Request block crc32.

		if (pseg->skip == 0)
		{ // Don't skip crc check	
print_state();printf("CRC-32 CHECK BLOCK: send crc request: pstart: %08X  pend: %08X\n", pseg->blkstm32_pstart, pseg->blkstm32_pend);	
			sendCMD_crc32(pseg->blkstm32_pstart, pseg->blkstm32_pend);	// Send crc32 request
			msg_p_sw1 = 11;
			break;
		}

		// Here, force reflashing everything
print_state();printf("SKIP CRC-32 CHECK BLOCK: load block %08X\n", pseg->blkstm32_pstart);
		msg_p_sw1 = 12;
		sendCMD_25(LDR_SET_ADDR, pseg->blkstm32_pstart); // Set address pointer.
		break;

	case 11: // Checking for crc32 for this block
		if (checkCMDmsg(pcan, LDR_CRC, 8) != 0) break; // Return not expected msg
print_state();printf("GOT CRC-32. ");
		// Here: response has the STM32 CRC.  See if ours and theirs match.
		crc = crc32(0, pseg->blkpc_pstart, pseg->blkpc_bytct); // Compute our crc for this blk (zlib crc)
		if (pcan->cd.ui[1] == crc)	// Received crc same as ours?
		{ // Here, the STM32 is the same as the PC, so skip this block
printf("MATCH: skip sending block. ");
			if (eos_flag != 0) // End-of-Segment(image) flag
			{ // Here, we are at the end of the segment or image
printf(" Also end of segment. Get next segment. \n");
				msg_p_sw1 = 17;	// Idle
				msg_p_sw  = 17;	// Idle jic
				cmd_p_sw  = cmd_p_sw_ret;	// Set cmd switch to get next segment/image
				break;
			}
			// Here, not end of segment/image.  Step pointers.
printf("'blk_pointers_next' \n");		
			eos_flag = blk_pointers_next(pseg);	// Get start and end addresses for image in PC and STM32
			sendCMD_crc32(pseg->blkstm32_pstart, pseg->blkstm32_pend);	// Request send crc32 for these
			msg_p_sw1 = 11;	// Do next block
			break;
		}
		// Here, the blocks differ so we have to send it.
print_state();printf("CRC-32 MISMATCH: load block: SET ADDR: %08X\n", pseg->blkstm32_pstart);
			msg_p_sw1 = 12;
			sendCMD_25(LDR_SET_ADDR, pseg->blkstm32_pstart); // Set address pointer.
		break;

	case 12: // Response to set address command
		if (checkCMDmsg(pcan, LDR_SET_ADDR, 2) != 0) break; // Return not expected msg
print_state();printf("Send high priority 'SQUELCH': tick ct: %d\n",WAITDELAY);
		/* Cause other units to squelch their CAN msg sending for WAITDELAY ticks */
		sendMASTERRESET_squelch(WAITDELAY);

		// Here, response to setting the address pointer
		if (pcan->cd.uc[1] != LDR_ACK)
		{ // Here, trouble.
print_state();printf("GOT A\"LDR_SET_ADDR\": ERROR: %d %08X Get next segment.\n", pcan->cd.uc[1], pseg->blkstm32_pstart);
			cmd_p_sw  = cmd_p_sw_ret;	// Get next 
			msg_p_sw  = 17;  // Idle
			msg_p_sw1 = 17;  // Idle
		}
		// Here, it passed the checks
print_state();printf("GOT A\"LDR_SET_ADDR\" and no errors.\n");
RcvMsgCtr = 0;
		eob_flag = setupandsend(pcan,pseg);	// Get 1st payload of flash block on its way.
		msg_p_sw1 = 14;	
		break;

	/* The following repeats until the end of a flash block, or end of segment/image */
	case 14: // Wait for ACK to send next msg
		if (checkCMDmsg(pcan, LDR_ACK, 1) != 0) 
		{ // Here, not ACK
			if (checkCMDmsg(pcan, LDR_NACK, 1) != 0) // Return not NACK or ACK
			{ printf("case 4: ############# NACK! ##############\n"); cmd_p_sw = cmd_p_sw_ret; break;}
		}
//printf("case 14: %d %08X %08X\n", RcvMsgCtr, blkstm32_pstart, blkpc_pstart);
	
		if (eob_flag == 0) 
		{
			eob_flag = setupandsend(pcan,pseg);
			break; // re-execute this 'case'
		}

		// Here, end-of-block msg was sent in the foregoing 'setupandsend'
print_state();printf("Last payload of block was sent: msgct: %d pstart: %08X dlc: %d\n", RcvMsgCtr, (u32)pseg->blkstm32_pstart, pcan->dlc);
		sendCMD_simple(LDR_WRBLK);	// Tell stm32 to write the flash block
		msg_p_sw1 = 9;			// Get next block
		break;	



	case 17: // Idle. Do nothing.
print_state();printf("IDLE: RCV'd msg: id: %08X dlc: %i ui[0]: %08X ui[1]: %08X\n",pcan->id, pcan->dlc, pcan->cd.ui[0], pcan->cd.ui[1]);
		break;

	default: printf("BOGUS msg_p_sw1 value: %d\n",msg_p_sw1);
		break;

	}
	return;
}
/******************************************************************************
 * static int setupandsend(struct CANRCVBUF* pcan, struct PSEG* pseg);
 * @brief 	: Setup and send "WR" CAN msg with payload loaded with PC image data
 * @param	: pcan = pointer to CAN msg
 * @param	: pseg = pointer struct with pointers to image and stm32 address
 * @return	: End of block status--	
 *		:  0 = 8 byte payload and not end of block
 *		: -1 = This msg ends the segment/image
 *		: +1 = This msg ends a block 
*******************************************************************************/
static int setupandsend(struct CANRCVBUF* pcan, struct PSEG* pseg)
{
	int ret;

	/* Will an LDR_WRVAL_PTR_SIZE (= 7) byte payload exceed the end of the image data? */
	if ((pseg->blkstm32_pstart + LDR_WRVAL_PTR_SIZE) >= pseg->addr_end)
	{ // Here, yes.  A partial payload will be sent.
		pcan->dlc = (pseg->addr_end - pseg->blkstm32_pstart);
		ret = -1;	// This msg ends sending the segment/image					
	}
	else
	{	/* Will an LDR_WRVAL_PTR_SIZE (= 7) byte payload exceed the end of the current block? */
	 	if ((pseg->blkstm32_pstart + LDR_WRVAL_PTR_SIZE) >= pseg->blkstm32_bend)
		{ // Here, yes.  A partial payload will be sent.
			pcan->dlc = (pseg->blkstm32_bend - pseg->blkstm32_pstart);
			ret = 1;	// This msg ends the current block
		}	
		else
		{  // Here, no.  Send a full payload
			pcan->dlc = LDR_WRVAL_PTR_SIZE;
			if (pseg->blkstm32_pstart >= pseg->blkstm32_bend)
				ret = 1;
			ret = 0;	// More to send in this current block					
		}
	}
//printf("\t\tpstart: %08X addr_end: %08X bend: %08X dlc: %d ret: %i\n",pseg->blkstm32_pstart,pseg->addr_end,pseg->blkstm32_bend,pcan->dlc, ret);
	memmove(&pcan->cd.uc[1], pseg->blkpc_pstart, pcan->dlc);
	pseg->blkstm32_pstart += pcan->dlc;	// Advance working pointers
	pseg->blkpc_pstart    += pcan->dlc;

	/* Finish CAN msg setup and send. */
	pcan->dlc += 1;		// Allow for command code
	pcan->cd.uc[0] = LDR_WRVAL_PTR;	// Command code to write payload
	sendcanmsg(pcan);	// Send msg with payload
	return ret;
}
/******************************************************************************
 * static int blk_pointers_first(struct PSEG* pseg);
 * @brief 	: Set working pointers for send the *first* block of the image
 * @param	: pseg = pointer...duh
 * @return	: End-of-Segment: 0 = not EOS; -1 = End of Segment(image)
*******************************************************************************/
static int blk_pointers_first(struct PSEG* pseg)
{
/* The start address in the stm32 might not begin on an even block boundary.  For
   programs it will, however for high flash areas it may not. */
	pseg->blkstm32_base  = pseg->blkstm32_pstart & ~(flash_blk_size - 1); // Base address of STM32 flash block
	pseg->blkstm32_bend  = pseg->blkstm32_base + flash_blk_size;	// End (so don't have to keep recomputing)
//printf("pstart: %08X ~fb: %08X fbz: %08x base: %08X bend: %08X\n",pseg->blkstm32_pstart, ~(flash_blk_size - 1),flash_blk_size,pseg->blkstm32_base, pseg->blkstm32_bend);
	pseg->blkpc_pstart = pseg->pb;	// The image in the PC is not block oriented.

	/* blkstm32_pend is needed for crc-32 requests */
	if (pseg->blkstm32_bend > pseg->addr_end) // Does data end before the next block starts?
	{ // Here, data ends before start of next block
		pseg->blkstm32_pend = pseg->addr_end;
		return -1;
	}
	// Here, there the data goes beyond the current block. 
	pseg->blkstm32_pend = pseg->blkstm32_bend;	// Limit sending to just the current block.
	
	return 0;
}
/******************************************************************************
 * static int blk_pointers_next(struct PSEG* pseg);
 * @brief 	: Set working pointers for send the *next* block of the image
 * @param	: pseg = pointer...duh
 * @return	: End-of-Segment: 0 = not EOS; -1 = End of Segment(image)
*******************************************************************************/
/* Note: 'blkstm32_pend' is only used in crc-32 request. */
static int blk_pointers_next(struct PSEG* pseg)
{
	/* Step block address pointers to next block. */
	pseg->blkstm32_base += flash_blk_size; 	// Beginning of block base address
	pseg->blkstm32_bend += flash_blk_size;	// End+1 (start of next block) address
	
	/* Check working addresses for end of image */
	if (pseg->blkstm32_bend > pseg->addr_end) // Does data extend beyond this block?
	{ // Here, data ends before start of next block
		pseg->blkstm32_pend = pseg->addr_end;
		return -1;
	}
	// Here, the data goes beyond the current block. 
	pseg->blkstm32_pend = pseg->blkstm32_bend;	// Limit sending to just the current block.
	
	return 0;
}
/******************************************************************************
 * static void print_state(void);
 * @brief 	: printf the states
*******************************************************************************/
static void print_state(void)
{
	printf("STATE: %02d %02d %02d %02d: SM: %02d: RM: %02d: ", cmd_p_sw, msg_p_sw, msg_p_sw1, cmd_p_sw_ret, SendMsgCtr,RcvMsgCtr);
	return;
}
/******************************************************************************
 * static int dlc_check(struct CANRCVBUF* pcan, u32 dlc);
 * @brief 	: Check dlc and printf err msg
 * @param	: Pointer to CAN msg
 * @param	: Expected dlc
 * @return	: 0 = OK; -1 = err
*******************************************************************************/
static int dlc_check(struct CANRCVBUF* pcan, u32 dlc)
{
	if ((pcan->dlc & 0xf) != dlc)
	{ print_state(); printf("CAN DLC %d does not match expected DLC %d\n",pcan->dlc, dlc); return -1;}
	return 0;
}

/******************************************************************************
 * unsigned int Crc32Fast(unsigned int* pData, unsigned int intcount);
 * @brief 	: Compute CRC-32 in same manner as STM32 hardware (i.e. 32b words)
 * @param	: pData = pointer words 
 * @param	: intcount = count of 4 byte words (WORDS NOT BYTES)
*******************************************************************************/
unsigned int Crc32Fast(unsigned int* pData, unsigned int intcount)
{
  static const unsigned int CrcTable[16] = { // Nibble lookup table for 0x04C11DB7 polynomial
    0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
    0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };

	int k;
	unsigned int Crc = ~0;

 	for (k = 0; k < intcount; k += 1)
	{	
		Crc = Crc ^ *pData++; // Apply all 32-bits 
 		// Process 32-bits, 4 at a time, or 8 rounds
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
		  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
 	}
  	return(Crc);
}
/******************************************************************************
 * static void flashpsend_init(void);
 * @brief 	: Set up struct for 'flashblk' routine to send an image of flashp
*******************************************************************************/
static void flashpsend_init(void)
{
	psegflashp.pb         = (unsigned char*)&bp[0];		// Point into binary image in this machine
	psegflashp.skip       = 0;				// Skip crc check (send all)
	psegflashp.addr_start = flashp_addr;			// Beginning stm32 address
	psegflashp.addr_end   = flashp_addr + flashp_size;	// (Ending + 1) stm32 address 

	psegflashp.blkstm32_pstart =  flashp_addr;
	psegflashp.blkstm32_pend   =  flashp_addr + flashp_size;

	psegflashp.blkpc_pstart = (unsigned char*)&bp[0];	// PC Pointer to start of blk
	psegflashp.blkpc_pend   = (unsigned char*)&bp[0] + flashp_size;	// Pointer into pc image start
	psegflashp.blkpc_bytct  = flashp_size;			// PC byte count

	msg_p_sw1 = 10;
	return;
}
/******************************************************************************
 * static void flashhsend_init(void);
 * @brief 	: Set up struct for 'flashblk' routine to send an image of flashh
*******************************************************************************/
static void flashhsend_init(void)
{
	psegflashh.pb         = (unsigned char*)&highflashh;	// Point into binary image in this machine
	psegflashh.skip       = ldp[loadpathfileIdx].force;	// Skip crc check (send all) if not zero
//psegflashh.skip       = 1;	// Debug Skip crc check (send all) if not zero
	psegflashh.addr_start = fixedaddr;			// Beginning stm32 address
	psegflashh.addr_end   = fixedaddr + sizeof(struct HIGHFLASHH); // (Ending + 1) stm32 address

	psegflashh.blkstm32_pstart =  fixedaddr;
	psegflashh.blkstm32_pend   =  fixedaddr + sizeof(struct HIGHFLASHH); // (Ending + 1) stm32 address
	
	psegflashh.blkpc_pstart = (unsigned char*)&highflashh;	// PC Pointer to start of blk
	psegflashh.blkpc_pend   = (unsigned char*)&highflashh + sizeof(struct HIGHFLASHH);	// Pointer start address of struct in PC
	psegflashh.blkpc_bytct  = sizeof(struct HIGHFLASHH);	// PC byte count

	msg_p_sw1 = 10;
	return;
}

