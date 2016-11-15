/******************************************************************************
* Date First Issued  : 02/06/2014
* Board              : PC
* Description        : histogram readout from shaft sensor
*******************************************************************************/
/*

02-07-2014 rev 193: Poll repsonse for shaft sensor

*/
#include "cmd_h.h"
#include "USB_PC_gateway.h"

/* Subroutine prototypes */
static void cmd_h_sendmsg(struct CANRCVBUF* p);

extern int fdp;	/* port file descriptor */

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

	if (strlen(p) < 8)
	{ // Here too few chars
		printf("Too few chars for the 'h' command (needs 8 for the id and one for mode), example\nh 31e00000 0 [as seen in the m command list]\n");
		return -1;
	}
	
	sscanf( (p+1), "%x %u",&keybrd_id, &mode);
	printf ("ID: %x %u, MODE: ",keybrd_id, mode);
	if (mode == 0) printf("HISTOGRAM\n");
	if (mode == 1) printf("ADC3 ADC2 filtered readings\n");
	if (mode > 1) {printf("mode must be 0 or 1\n"); return -1;}

	state = 0;	// Sequence through the readout.

	/* Prepare 1st msg ADC3 */
	canmsg1.id = ( (keybrd_id & CAN_UNITID_MASK) | CAN_DATAID29_1 | CAN_IDE);
	canmsg1.dlc = 4;
	canmsg1.cd.ull = 0; // jic

	/* Prepare ADC3 bin readout command msg */
	canmsg2.id = ( (keybrd_id & CAN_UNITID_MASK) | CAN_DATAID29_2 | CAN_IDE);
	canmsg2.dlc = 0;
	canmsg2.cd.ull = 0; // Initial bin # is zero.

	/* Prepare 1st msg ADC2 */
	canmsg3.id = ( (keybrd_id & CAN_UNITID_MASK) | CAN_DATAID29_3 | CAN_IDE);
	canmsg3.dlc = 4;
	canmsg3.cd.ull = 0; // jic

	/* Prepare ADC2 bin readout command msg */
	canmsg4.id = ( (keybrd_id & CAN_UNITID_MASK) | CAN_DATAID29_4 | CAN_IDE);
	canmsg4.dlc = 0;
	canmsg4.cd.ull = 0; // Initial bin # is zero.

	/* Command ADC3 ADC2 filtered readings readout */
	canmsg5.id = ( (keybrd_id & CAN_UNITID_MASK) | CAN_DATAID29_5 | CAN_IDE);
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

