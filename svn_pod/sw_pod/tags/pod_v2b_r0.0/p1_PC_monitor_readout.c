/******************************************************************************
* File Name          : p1_PC_monitor_readout.c
* Author             : deh
* Date First Issued  : 10/06/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output packet data to USART, and monitor 
*******************************************************************************/
#include "p1_common.h"
#include <string.h>
/*
This routine is polled from 'p1_PC_handler.c' when a 'r' command is in effect.
When the SD card readbackwards supplies a record it is checked by the case statement
for a match to one of the recognizes sizes.  The case, then calls a routine that
first checks that the ID as determined by the size matches the first char in the record.
It then uses the struct definition for that packet and produces an output that is in
ASCII/HEX.  If there is an error, there is an error line inserted before the line with
the (bogus?) data.
*/


/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_convert.h
@2 = svn_pod/sw_pod/trunk/pod_v1/common.h
@3 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_linux.h

*/

extern short state_PC_handler;	// State used in 'p1_PC_handler.c'

unsigned long long Debugreadout;


static int packet_setup(int temp, int size, int id);
static int packet_setup_gps(struct TIMESTAMPGP1 * timestampg,int temp, int id);
static int packet_setup_batt(struct PKT_BATTMP *batttmp, int temp, int id);
static int packet_setup_pushbutton(struct PKT_PUSHBUTTON *pushbutton, int temp, int id);
static int packet_setup_accel(struct PKT_ACCEL *accel, int temp, int id);
static int packet_setup_ad7799(struct PKT_AD7799 *pkt_ad7799, int temp, int id);
static int packet_setup_gpsfix(struct PKT_GPSFIX *pkt_gpsfix, int temp, int id);

static union PKT_ALL pkt_all;

static char packet_PC_buf[2*PACKETREADBUFSIZE];	// Hold ASCII output for PC (size defined in p1_common.h (@2))
static char packet_PC_err[128];		// Holds Error line
static char limit;			// Limit ERR msgs
static unsigned long ulpackettime;	// Latest read packet time

static const char * stopmsg = "< ##### Stop time has been reached at";
static const char * startmsg = " ##### Start time we are searching for is";
static const char * startmsgerr = "##### Error: Start time supplied is in future of now:";
static int throttle;	// Pace output of progress display when spinning towards readout start time
#define THROTTLECT 1538	// Count of packets between date/time displaying (approx 5 min steps)
/******************************************************************************
 * int p1_PC_monitor_readout(void);
 * @brief	: Output packets to PC
 * @return	: 0 = OK; 1 = all packets read
 ******************************************************************************/
static 	short state;
	short state_readout;	// Sub-sequence start|stop times data readout (for 'state = 2')

int p1_PC_monitor_readout(void)
{
	int temp;
	unsigned int uitemp;
	int code1 = 0;

	switch (state)
	{
	case 0:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return 0;		// Return: all buffers full (@1)
		
/* sdlog_read_backwards(...) reads the next packet from the input stream and 
 * puts the data into "buf".  If "count" is less than SDLOG_WRITE_MAX and the 
 * next packet size is greater than "count", then the "count" bytes of data 
 * are copied to "buf" and the remainder are discarded.  No error is signalled.
 * The size of the packet read is returned, or negative for error.  An attempt 
 * to read past the last lacket written is signalled by returning a 0.  An
 * attempt to read backwards past the first packet is signalled by returning 
 * a 0.  Note that the first read after restart 
 */
// int sdlog_read_backwards(char *buf, int buf_size);
		temp = sdlog_read_backwards(pkt_all.packet_read_buf, PACKETREADBUFSIZE);
Debugreadout += temp;

		if (temp <= 0) return 1; //code1 = 1;	// Tell PC_handler that the SD card readback is complete

		/* The following performs essentially a table lookup error check on length, then setup for output */
		switch (temp)
		{
		case sizeof (struct PKT_AD7799):	/* ID = 1 Tension packet */
				state = packet_setup_ad7799(&pkt_all.pkt_ad7799,temp,PKT_AD7799_ID);	// Setup for output			
				break;
		case sizeof (struct TIMESTAMPGP1):	/* ID = 2 GPS */
				state = packet_setup_gps(&pkt_all.timestampg,temp,PKT_GPS_ID);	// Setup for output
				break;
		case sizeof (struct PKT_BATTMP):	/* ID = 3 Battery/temp */
				state = packet_setup_batt(&pkt_all.batttmp,temp,ID_BATTTEMP);	// Setup for output
				break;
		case sizeof (struct PKT_PUSHBUTTON):	/* ID = 4 Pushbutton */
				state = packet_setup_pushbutton(&pkt_all.pushbutton,temp,PKT_PUSHBUTTON_ID);	// Setup for output
				break;
		case sizeof(struct PKT_ACCEL):		/* ID = 5 Accelerometer packet */
				state = packet_setup_accel(&pkt_all.accel,temp,ID_ACCEL);		// Setup for output
				break;
		case sizeof(struct PKT_GPSFIX):		/* ID = 6 GPS sentence packet */
				state = packet_setup_gpsfix(&pkt_all.gpsfix,temp,ID_GPSFIX);		// Setup for output
				break;
		default: 	/* All others show the error */
				if (limit < 4)
				{
					if (temp < 0)
					{
						sprintf (packet_PC_err,"ERR2 size: %9d 1st byte: %02x \n\r",temp,packet_PC_buf[1]);
						state = 4;
						limit += 1;
	
					}
					else
					{
						state = packet_setup(temp,temp,-1);		// Setup for output
					}
				}
				break;
		}
		break;

	case 2: /* Output packet in hex ascii */

		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return 0;		// Return: all buffers full (@1)

		/* Only send packets between a start and stop date/time if specified */

		/* ===> REMEMBER: readout of data works backward in time <==== */

		if (ulreadbackstart != 0) // Were start|stop times specified?
		{ // Yes.  Deal with the times
			switch (state_readout)	// This handles startup and termination of readout between start|stop date/times
			{
			case 0:	// The first time should be later than the start time
				if (ulreadbackstart  >= ulpackettime) 
				{ // Here the readout is further back in time than the start time specified.
					state_readout = 1;	// Reset readout position next pass
				}
				else
				{ // Here, we will not be starting in the middle of the start|stop times
					USART1_txint_puts((char*)startmsg); // Let hapless op see the date/time we are searching for

					/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
					uitemp = ulreadbackstart + PODTIMEEPOCH;
	
					/* Display the start time we search for in date/time format */
						printf ("  %s\r", ctime((const time_t*)&uitemp));		
					USART1_txint_send();
					state_readout = 2;
				}
					break;

			case 1: // Here reset readout position to current time
				{
					sdlog_seek(~0ULL);	// Reset readout back to start with most recent
					state_readout = 10;	// Re do this				
				}
				break;

			case 10: // After resetting the readout to "now" the first time should always be later than a valid start time
				if (ulreadbackstart  >= ulpackettime)
				{ // Here the readout is further back in time than the start time specified after re-positioning readout to "now"
					state_readout = 30;	// Give error message and quit 'd' (aliased to 'r') command
					break;
				}
				else
				{ // Here, we will not be starting in the middle of the start|stop times
					USART1_txint_puts((char *)startmsg); // Let hapless op see the date/time we are searching for

					/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
					uitemp = ulreadbackstart + PODTIMEEPOCH;
	
					/* Display the start time we search for in date/time format */
						printf ("  %s\r", ctime((const time_t*)&uitemp));		
					USART1_txint_send();
					state_readout = 2;
				}
					break;

			case 2: // Here readout began later than start time, so we are OK for the start time
				
				/* Only send packets between a start and stop date/time if specified */
				if (ulreadbackstart != 0) // Were start|stop times specified?
				{ // Here we have a start time, and stop time might be zero (in which case a manual stop is required ('x' cmd))
					if (ulreadbackstart  >= ulpackettime)
					{
//						if  (ulpackettime > 1000)
							USART1_txint_puts(packet_PC_buf); USART1_txint_send();//  (@1)
					}
					else
					{ // Here, we have not spun back to the start time 
						if (throttle++ >= THROTTLECT)
						{
							throttle = 0;
							/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
							uitemp = ulpackettime;
							uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch
	
							/* Display the packet date/time */
							printf ("  %s\r", ctime((const time_t*)&uitemp));		
							USART1_txint_send();
						}
					}
				}
				if  ((ulreadbackstop >= ulpackettime) && (ulpackettime > 100000))
				{ // Here the readout has past the stop time.  Time to end
					state_readout = 3;
				}
				break;

			case 30: // Here terminate readout because we hit the stop time
				if (USART1_txint_busy() == 1) return 0;		// Return: all buffers full (@1)
				
				/* Output error message */
				USART1_txint_puts((char *)startmsgerr); 
	
				/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
				uitemp = ulreadbackstart;
				uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

				/* Display the packet time that we stopped on in date/time format */
				printf ("  %s\r\n\r", ctime((const time_t*)&uitemp));		
				USART1_txint_send();
				
				/* Initialize for a new 'd' command */
				state_PC_handler = 0; 	// Reset 'r' ('d') command which causes menu to print
				ulreadbackstart = 0;	// JIC
				state_readout = 0;	// Reset for new 'd' command
				break;

			case 3: // Here terminate readout because we hit the stop time
				if (USART1_txint_busy() == 1) return 0;		// Return: all buffers full (@1)
				
				/* This terminating message has '<' as the 1st char which will flag the PC to close the file.  BTW, '>' 
				   flags the PC that the POD is ready for a 'd' command. */
				USART1_txint_puts((char *)stopmsg); 
	
				/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
				uitemp = ulreadbackstop;
				uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

				/* Display the packet time that we stopped on in date/time format */
				printf ("  %s\r\n\r", ctime((const time_t*)&uitemp));		
				USART1_txint_send();
				
				/* Initialize for a new 'd' command */
				state_PC_handler = 0; 	// Reset 'r' ('d') command which causes menu to print
				ulreadbackstart = 0;	// JIC
				state_readout = 0;	// Reset for new 'd' command
			}

		}
		else
		{ // Here, there are no start|stop times, so send all packets out.
			USART1_txint_puts(packet_PC_buf);		USART1_txint_send();//  (@1)
		}
		state = 0;	// Back to beginning
		break;

	case 3: /* Error.  Put out the error line first*/
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return 0;		// Return: all buffers full (@1)
		
		/* Send output error line to the USART routine */
		USART1_txint_puts(packet_PC_err);		USART1_txint_send();//  (@1)

		/* Now, output the packet */
		state = 2;		
		break;

	case 4:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return 0;		// Return: all buffers full (@1)
		
		/* Send output error line to the USART routine */
		USART1_txint_puts(packet_PC_err);		USART1_txint_send();//  (@1)

		/* Back to beginning */
		state = 0;		
	}
	
	return code1;
}

/******************************************************************************/
/* static char* hexchar (char* pout, char pin);                                */
/******************************************************************************/
static const char hextbl[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
static char* hexchar (char* pout, char in)
{
	*pout++ = hextbl[ ( (in >> 4) & 0x0f )];
	*pout++ = hextbl[  (in & 0x0f) ];
	return pout;

}
/******************************************************************************/
/* static char* hexshort (char* pout, short s);                                */
/******************************************************************************/
static char* hexshort (char* pout, short s)
{
	pout = hexchar(pout, ( (char)((s >> 8) & 0xff) ));
	pout = hexchar(pout,   (char)(s & 0xff) );
	return pout;
}
/******************************************************************************/
/* static char* hexlong (char* pout, long l);                                */
/******************************************************************************/
static char* hexlong (char* pout, long l)
{
	pout = hexshort(pout, ( (short)((l >> 16) & 0xffff) ));
	pout = hexshort(pout,   (short)(l & 0xffff) );
	return pout;
}
/******************************************************************************/
/* static char* hexlonglong (char* pout, unsigned long long ll);              */
/******************************************************************************/
//static char* hexlonglong (char* pout, unsigned long long ll)
//{
//	pout = hexlong(pout, ( (short)((ll >> 32) & 0xffffffff) ));
//	pout = hexlong(pout,   (short)(ll & 0xffffffff) );
//	return pout;
//}
/******************************************************************************
 * static char* hex_n (char* pout, int n, union LL_L_S * pll);                
 * @brief	: Starting with high order byte convert n bytes to hex output
 * @param	: pointer to output buffer receiving hex chars
 * @param	: count of input bytes to hex, starts 
 * @param	: union to ties unsigned 64b, 32b, 16b, 8b together
******************************************************************************/
static char* hex_num(char* pout, int n, union LL_L_S * pll)
{
	int i;

	for ( i = 0; i < n; i++)	// Low ord -> Hi ord
	{
		pout = hexchar(pout,pll->uc[n-i-1]);		
	}
	return pout;
}
/******************************************************************************
 * static char* hex_tickct (char* pout, union LL_L_S * pll);                
 * @brief	: Starting with high order byte convert n bytes to hex output
 * @param	: pointer to output buffer receiving hex chars
 * @param	: union to ties unsigned 64b, 32b, 16b, 8b together
******************************************************************************/
static unsigned long long last_tickct;

static char* hex_tickct (char* pout, union LL_L_S * pll)                
{
	/* Save packet time for logic that by-pass times outside of start|stop times */
	ulpackettime = (pll->ull >> 11);

	/* Shift time epoch from linux 1/1/1970 to pod (10/13/2011) (@3) */
	pout = hex_num(pout,PODTIMEEPOCHBYTECT,pll);	// Convert 5 bytes to hex 
	last_tickct = pll->ull;	// Save the latest tick ct for display purposes (cmd == '*')
	return pout;
}
/******************************************************************************/
/* static int packet_setup(int temp, int size, int id)                        */
/* This is a raw hex'ing of the struct (maybe not used)			      */
/******************************************************************************/
static int packet_setup(int temp, int size, int id)
{
	char* pout = &packet_PC_buf[1];		// ID gets handled separately
	char* pin  = &pkt_all.packet_read_buf[4];	// ID occupies 4 bytes, but only one significant
	int code2 = 2;
	int i;

	if (pkt_all.packet_read_buf[0] != id)	// Does the ID presumed from the packet size match the ID in the packet?
	{ // Here, no.  Size and ID do not match
		sprintf (packet_PC_err,"ERR1 size: %5u id: %5u 1st byte: %5u \n\r",temp,id,pkt_all.packet_read_buf[0]);
		code2 = 3;
	}
	packet_PC_buf[0] = id+'0';	// Only one byte needed for packet ID

	if (size > PACKETREADBUFSIZE-1) size = PACKETREADBUFSIZE-1;
	for (i = 0; i < size-3; i++)	
	{
		pout = hexchar(pout,*pin++);
	}
	*pout++ = '\n';	*pout++ = '\r'; *pout = 0;	// Add new line and string terminator

	return code2;
}
/******************************************************************************
* static int idcheck(int temp, int id);                                      
* @brief	: Compare id to 1st char of packet record
* @param	: size of packet, as returned from sd card read
* @param	: id as determined from packet size
* @return	: 2 = send line usart; 3 = send error msg, then line to usart
******************************************************************************/
static int idcheck(int temp, int id)
{
	int code = 2;

	if (pkt_all.packet_read_buf[0] != id)	// Does the ID presumed from the packet size match the ID in the packet?
	{ // Here, no.  Size and ID do not match
		sprintf (packet_PC_err,"ERR1 size: %5u id: %5u 1st byte: %5u \n\r",temp,id,pkt_all.packet_read_buf[0]);
		code = 3;
	}
	return code;
}
/* ============================== GPS ID 2 ================================================== */
/******************************************************************************
* static int packet_setup_gps(struct TIMESTAMPGP1 * timestampg, int temp, int id)                             
* @brief	: Convert ascii & binary packet to hex output to USART, strip padding           
* @param	: struct used to write packet
* @param	: temp = size of record as given by sd card read routine
* @param	: id as presumed from size of packet record
******************************************************************************/
static int packet_setup_gps(struct TIMESTAMPGP1 * timestampg,int temp, int id)
{
	unsigned int i;				// Famous FORTRAN integer variable
	int code   = idcheck(temp,id);		// Check that id code based on size matches record
	char* pout = &packet_PC_buf[0];		// Hex output buffer pointer

	*pout++ = id+'0';			// Convert ID byte to ASCII
	pout = hex_tickct(pout,&timestampg->alltime.SYS);// Convert tick count to hex (@1)

/* NOTE: At some point after rev 350 the "12" in the 'for' loop was replaced with 
"#define GPSARRAYSIZE	14" that is in 'p1_common.h'.  This copied a null, which then
causes the output to the USART to stop short of the '\n\ and '\r', with the result
that when GPS is present the tension packet output that follows this gps packet is
missing appended to the this packet and that makes the tension packet appear to be
missing later in the post-processing routines. */

	for ( i = 0; i < 12; i ++)	// Copy ASCII gps time array (@1)
	{

	// The following 'if' was put in during debugging the above problem.  We'll just leave it jic.
		if (timestampg->g[i] == 0) // If we encounter a '\0' the CR/LF will not appear in the output
			*pout++ = '%';	// So substitute some ascii char 
		else
			*pout++ = timestampg->g[i];	// Time field
	}

	*pout++ = '\n';	*pout++ = '\r';	*pout++ = 0;	// Add new line and string terminator

	return code;
}
/* ======================= ADC: BATTTMP ID 3 ==================================================== */
/*****************************************************************************************
Setup for output an unsigned with a 100* scale as xxxx.xx
*****************************************************************************************/
static char* output_100_scale(char *pout, int uiS)
{
	sprintf (pout, "%5d.%02u ",uiS/100,(uiS % 100) );
	pout += 9;
	return pout;
}
/*****************************************************************************************
Setup an int for a floating pt type output
*****************************************************************************************/
static char* output_calibrated_adc (char *pout, unsigned int uiT)
{
	unsigned int uiX = uiT/1000;	// Whole part
	unsigned int uiR = uiT % 1000;	// Fractional part

	sprintf (pout, " %3u.%03u  ",uiX,uiR);// Setup the output in a buffer
	pout += 9;

	return pout;
}

/******************************************************************************
* static int packet_setup_batt(struct PKT_BATTMP batttmp, int temp, int id)
* @brief	: Convert ascii & binary packet to hex output to USART, strip padding           
* @param	: struct used to write packet
* @param	: temp = size of record as given by sd card read routine
* @param	: id as presumed from size of packet record
******************************************************************************/
static int packet_setup_batt(struct PKT_BATTMP *batttmp, int temp, int id)
{
/* Much of this code was lifted from 'p1_PC_monitor_batt.c' which in turn was lifted from 'adctest.c'*/

	/* Vars for conversion of ADC readings of thermistor  */
	unsigned int uiThermtmp;
	unsigned int uiAdcTherm;
	unsigned int uiAdcTherm_m;
	unsigned int uiThermtmpF;
	unsigned int uiBotCellCal;
	unsigned int uiTopCellCal;
	unsigned int uiTopCellDif;

	int code   = idcheck(temp,id);		// Check id code match
	char* pout = &packet_PC_buf[0];		// Hex output buffer

//*pout++ = '@';
	*pout++ = id+'0';			// Convert ID byte to ASCII

	pout = hex_tickct(pout,&batttmp->U); 		// Convert tick count to hex


	/* Output current battery voltages Top, Bottom cell, and difference, which is the Top cell */
			
	/* Calibrate & scale voltages to (volts * 100) */

	uiBotCellCal = ( strDefaultCalib.adcbot * batttmp->adc[1] )>>18;
	uiTopCellCal = ( strDefaultCalib.adctop * batttmp->adc[2] )>>18;
	uiTopCellDif = uiTopCellCal - uiBotCellCal;	// Top cell = (Top of battery volts - lower cell volts)


// Debug: show raw adc readings
// printf ("%6u %6u \n\r",(batttmp..adc[1] ),(batttmp..adc[2]) );
// printf ("%6u %6u ",batttemp_monbuf.adc[1],batttemp_monbuf.adc[2]);
	
	/* Setup voltages for output */
	pout = output_calibrated_adc(pout,  uiBotCellCal );	// Top of battery
	pout = output_calibrated_adc(pout,  uiTopCellCal );	// Bottom cell
	pout = output_calibrated_adc(pout,  uiTopCellDif );	// Top cell
	
	uiAdcTherm_m = batttmp->adc[0]>>2;
	
	/* Average thermistor ADC reading (xxxxxx.x) */
	sprintf(pout," %6d ",uiAdcTherm_m );  pout += 8;
	
	/* Display the thermistor temperature in C and F */
	uiAdcTherm = uiAdcTherm_m;		// 
	uiThermtmp = adctherm_cal(uiAdcTherm);	// Convert ADC reading to temp (deg C)
	pout = output_100_scale(pout, uiThermtmp+strDefaultCalib.tmpoff);// Output as XXXX.XX
	uiThermtmpF = ((uiThermtmp+strDefaultCalib.tmpoff)*9)/5 + 3200;	// Convert to Farenheit
	pout = output_100_scale(pout, uiThermtmpF);			// Output as XXXX.XX

	*pout++ = '\n';	*pout++ = '\r'; *pout = 0;	// Add new line and string terminator

	return code;
}

/* =========================== PKT_PUSHBUTTON Pushbutton ID 4 =================================================== */
/******************************************************************************
* static int packet_setup_pushbutton(struct PKT_PUSHBUTTON *pushbutton, int temp, int id)
* @brief	: Convert ascii & binary packet to hex output to USART, strip padding           
* @param	: struct used to write packet
* @param	: temp = size of record as given by sd card read routine
* @param	: id as presumed from size of packet record
******************************************************************************/
static int packet_setup_pushbutton(struct PKT_PUSHBUTTON *pushbutton, int temp, int id)
{
	int code   = idcheck(temp,id);		// Check id code match
	char* pout = &packet_PC_buf[0];		// Hex output buffer

//*pout++ = '*';
	*pout++ = id+'0';			// Convert ID byte to ASCII

	
	pout = hex_tickct(pout,&pushbutton->U); 	// Convert tick count to hex

	*pout++ = '\n';	*pout++ = '\r'; *pout = 0;	// Add new line and string terminator

	return code;
}
/* =========================== ADC: PKT_ACCEL accelerometer ID 5 =================================================== */
/******************************************************************************
* static int packet_setup_accel(struct PKT_ACCEL *accel, int temp, int id)
* @brief	: Convert ascii & binary packet to hex output to USART, strip padding           
* @param	: struct used to write packet
* @param	: temp = size of record as given by sd card read routine
* @param	: id as presumed from size of packet record
******************************************************************************/
static int packet_setup_accel(struct PKT_ACCEL *accel, int temp, int id)
{
	int	nT;	// Temp
	short	offset;	// Hold for 'j' loop
	short	scale;	// Hold for 'j' loop
	int	i,j;	// for indexing for FORTRAN type loops
	
	int code   = idcheck(temp,id);		// Check id code match
	char* pout = &packet_PC_buf[0];		// Set ptr to beginning of Hex output buffer

//*pout++ = '$';
	*pout++ = id+'0';			// Convert ID byte to ASCII

	
	pout = hex_tickct(pout,&accel->U); 		// Convert tick count to hex

	/* Calibrate accelerometer: Z,Y,X into g-force as signed, 16 bit  */

	for ( i = 0; i < NUMBERACCELREADINGSPKT; i ++)		// Go through each axis
	{
		offset = strDefaultCalib.accel_offset[i];	// Offset calibration for this axis
		scale = strDefaultCalib.accel_scale[i];		// Scale calibration for this axis

		for ( j = 0; j < PKT_ACCEL_GRP_SIZE; j++)	// Rattle through all the readings 
		{
			nT = accel->adc[i][j] - offset;		// Adjust for zero point
			nT = (nT*1000)/scale;			// Scale to g * 100
			pout = hexshort(pout,(short)(nT & 0xffff) );	// Output as hex
		}
	}


	*pout++ = '\n';	*pout++ = '\r'; *pout = 0;	// Add new line and string terminator

	return code;
}
/* =========================== AD7799 Tension ID 1 =================================================== */
/******************************************************************************
*static int packet_setup_ad7799(struct PKT_AD7799 *pkt_ad7799, int temp, int id)
* @brief	: Convert ascii & binary packet to hex output to USART, strip padding           
* @param	: struct used to write packet
* @param	: temp = size of record as given by sd card read routine
* @param	: id as presumed from size of packet record
******************************************************************************/
static int packet_setup_ad7799(struct PKT_AD7799 *pkt_ad7799, int temp, int id)
{
	int	ntemp;		// Used in calibration 
	long long lltemp;	// Used in calibration 
	int	i;		// Nice FORTRAN name for a loop
	
	int code   = idcheck(temp,id);		// Check id code match
	char* pout = &packet_PC_buf[0];		// Hex output buffer pointer

	*pout++ = id+'0';			// Convert ID byte to ASCII
	
	pout = hex_tickct(pout,&pkt_ad7799->U); 		// Convert tick count to hex

	/* Output to the PC  */

	for ( i = 0; i < PKT_AD7799_TENSION_SIZE; i ++)		// Go through the readings contained in this packet
	{
		/* Slight adjustment of the load_cell and ad7799 offset for zero */
		lltemp = pkt_ad7799->tension[i] + strDefaultCalib.load_cell_zero;

		/* Convert to kgs * 1000 (which I supposed one might call "grams"!) */
		lltemp = lltemp*10000 / strDefaultCalib.load_cell;	// Gain scaling
		ntemp = (int)lltemp;	// Convert back to signed 32 bits

		pout = hexlong(pout, ntemp);	// Output as hex
	}

	*pout++ = '\n';	*pout++ = '\r'; *pout = 0;	// Add new line and string terminator

	return code;
}
/******************************************************************************
* void p1_PC_monitor_readout_datetime(void);
* @brief	: Display date/time and pause readback operation
******************************************************************************/
void p1_PC_monitor_readout_datetime(void)
{
		unsigned int uitemp = last_tickct >> ALR_INC_ORDER;

		/* Get epoch shifted out of our hokey scheme for saving a byte to the 'ctime' routine basis */
		uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

		/* Convert to ascii */
		printf ("  %s\r", ctime((const time_t*)&uitemp));	USART1_txint_send();
		
	return;
}

/* =========================== GPSFIX gps sentence ID 6 =================================================== */
/******************************************************************************
*static int packet_setup_gpsfix(struct PKT_GPSFIX *pkt_gpsfix, int temp, int id);
* @brief	: Convert ascii & binary packet to hex output to USART, strip padding           
* @param	: struct used to write packet
* @param	: temp = size of record as given by sd card read routine
* @param	: id as presumed from size of packet record
******************************************************************************/
static int packet_setup_gpsfix(struct PKT_GPSFIX *pkt_gpsfix, int temp, int id)
{
	unsigned int	j = 0;		// JIC terminator is missing

	int code   = idcheck(temp,id);		// Check id code match
	char* pout = &packet_PC_buf[0];		// Hex output buffer pointer
	char* pin = &pkt_gpsfix->c[0];	// Input chars in gps sentence

	*pout++ = id+'0';			// Convert ID byte to ASCII
	
	/* Time stamp with the Extended Linux time */
	pout = hex_tickct(pout,&pkt_gpsfix->U); // Convert tick count to hex

	/* Output to the PC  */
	// Copy until--hit size limit, or zero terminator, or LF, or CR 
	while ( ((j++ < sizeof (struct PKT_GPSFIX)) && (*pin != 0) && (*pin != 0x0d) && (*pin != 0x0a)) ) *pout++ = *pin++;

	*pout++ = '\n';	*pout++ = '\r'; *pout = 0;	// Add new line and string terminator

	return code;
}

