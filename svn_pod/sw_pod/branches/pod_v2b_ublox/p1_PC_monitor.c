/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_monitor.c
* Author             : deh
* Date First Issued  : 09/10/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current data to PC
*******************************************************************************/
/*
This routine "monitors" the current data, by outputting the readings in ASCII to the
PC.  It uses the same tension reading buffers as used for the SD Card writing, but
the index for retrieving the readings is separate from the one used with the main
polling loop for writing the SD card.

p1_PC_handler.c polls this routine if a 'm' command has been entered.  The polling
continues until a 'x' command is received.

The data 
*/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/lib/libsupport/gps_packetize.h


*/

#include "p1_common.h"	// Just about everything is in this one

#define	SECONDFILTERDECIMATE	16	// Second CIC filter decimation 
#define SECONDFILTERSCALE	15	// Scale factor: number of right shifts

/* Toggles load_cell zeroing on/off when the 'm' command is in effect */
extern unsigned int uiLoad_cell_zeroing;	// 0 = OFF, not-zero = ON.
extern unsigned int Debugtoggle;



/******************************************************************************
 * void p1_PC_monitor(void);
 * @brief 	: Output current tension data to PC
 * @param	: 0 = run; 1 = stop
*******************************************************************************/
static short state;
static unsigned short usPktIdx;		// Index of entry within a packet
static struct PKT_AD7799 * pp_ad7799;	// Pointer to tension packet being outputted
static struct CICLLN2M3 strMonad7799;	// Further filtering to slow down monitored tension data 

/* For getting the zero for the load_cell */
static long long llZero;	// Accumulator for average
static int 	nZeroctr;	// Count going into average
static int	nZeroave;	// Current average

void p1_PC_monitor(void)
{
	struct PKT_PTR pp_mon;		// Ptr & ct to current
	long long lltemp;
	int temp;
	int nT,nX,nR;


	if (strMonad7799.usDecimateNum == 0)	// Did we initialize the struct?
	{ // No.
		strMonad7799.usDecimateNum = SECONDFILTERDECIMATE;// Set decimation count
	}

	switch (state)
	{
	case 0:
		pp_mon = ad7799_packetize_get_monitor();	// Check if packet ready
		if (pp_mon.ct > 0)				// Skip if no bytes
		{
			usPktIdx = 0;	// Index for the reading entries within the packet
			pp_ad7799 = (struct PKT_AD7799 *)pp_mon.ptr;	// Convert Char pointer to struct pointer
			state = 1;	// Next state
			return;
		}
		return;

	case 1:

		/* Run the data through the second filter */
		strMonad7799.nIn = pp_ad7799->tension[usPktIdx];	// Load input data
		if (cic_filter_ll_N2_M3 (&strMonad7799) == 0)
		{ // Here, filter does not have an output for us
			usPktIdx++;
			if (usPktIdx >= PKT_AD7799_TENSION_SIZE) // Keeping chucking out readings until end of packet
			{
				state = 0;
				return;
			}			
		}
		else
		{ // Here, the filter has completed
			state = 2;
		}
		break;
			
	case 2:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;	// Return if all buffers full (@1)

		temp = strMonad7799.llout>>SECONDFILTERSCALE;	// Scale for filter gain
	
		/* Slight adjustment of the load_cell and ad7799 offset for zero */
		temp = temp + strDefaultCalib.load_cell_zero;

		/* Build an average that will be used for the kg scaled output */
		if (uiLoad_cell_zeroing != 0)	// Did the op type in a 'z' toggling this ON?
		{ // Yes, toggled ON
			llZero += temp;			// Accumulation of readings
			nZeroctr += 1;			// N in the average
			nZeroave = llZero/nZeroctr;	// Average
		}
		else
		{ // When toggled OFF restart for a new average
			llZero = 0; nZeroctr = 0;
		}

		/* Convert to kgs * 1000 and output as xxxxx.xxx */
		lltemp = temp - nZeroave;
		lltemp = lltemp*10000 / strDefaultCalib.load_cell;
		nT = (int)lltemp;

		/* If 'p' commmand was made, then make an adjustment to the zero in the calibration table */
		if (uiLoad_cell_zero_save != 0) // 'p' command flag set?
		{ // Here, yes.  Update the load-cell offset
			uiLoad_cell_zero_save = 0; 	// Just once.  We don't want to make this cumulative!
			strDefaultCalib.load_cell_zero = strDefaultCalib.load_cell_zero - nZeroave; // Update in-memory calibration
			nZeroave = 0;		// The offset adjustment is now in the calibration table so we don't need it.
			cCalChangeFlag = 1;	// We changed the calibration so set flag to cause update when we shutdown ('s' command)
			USART1_txint_puts ("Load-cell zero-offset update\n\r"); USART1_txint_send();
		}
		
		/* Rather ugly mess for printf'ing signed x.xxx without using floating pt routines */
			if (nT < 0)
			{
				nT = -nT;				// Deal with as positive for now
				nX = nT/1000;				// Whole part
				nR = nT % 1000;				// Fractional part
				if (nX == 0)
				{	
					printf ("       -0.%03u ",nR);	// Zero or less and negative
				}	
				else
				{
					printf ("%9d.%03d ",-nX,nR);	// Negative and greater than 0
				}
			}
			else
			{ // Positive values are easier 
				nX = nT/1000;				// Whole part
				nR = nT % 1000;				// Fractional part
				printf ("%9d.%03u ",nX,nR);		// Setup the output
			}



		printf("%9d", temp);	// Display uncalibrated reading (but with offset applied)

		/* When the 'z' command toggles, display the accumulation of the average offset to acheive zero net */
		if (uiLoad_cell_zeroing != 0)
		{
			printf (" %4u %6d",nZeroctr,nZeroave); // Count of the average, average

		}

		USART1_txint_puts ("\n\r");
		
		USART1_txint_send();	//  (@1)
		usPktIdx++;
		if (usPktIdx >= PKT_AD7799_TENSION_SIZE) // Keeping chucking out readings until end of packet
		{
			state = 0;
			return;
		}
	}
	return;
}


