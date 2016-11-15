/******************************************************************************
* File Name          : tension_a_functionS.c
* Date First Issued  : 05/29/2016
* Board              : f103
* Description        : Tension function_a for both AD7799s  Capital "S" for plural!
*******************************************************************************/

#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "tension_idx_v_struct.h"
#include "db/gen_db.h"
#include "tim3_ten2.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "tension_a_functionS.h"
#include "adcsensor_tension.h"
#include "temp_calc_param.h"
#include "PODpinconfig.h"
#include "poly_compute.h"
#include "p1_initialization.h"
#include "can_driver_filter.h"
#include "cmd_code_dispatch.h"

#define TENAQUEUESIZE	3	// Queue size for passing values between levels

/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to flash area with parameters  */
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)
extern void* __paramflash1;	// High flash address of 1st parameter table (.ld defined)
extern void* __paramflash2;	// High flash address of 2nd parameter table (.ld defined)

/* Holds parameters and associated computed values and readings for each instance. */
struct TENSIONFUNCTION ten_f[NUMTENSIONFUNCTIONS];

const uint32_t* pparamflash[NUMTENSIONFUNCTIONS] = { \
 (uint32_t*)&__paramflash1,
 (uint32_t*)&__paramflash2,
};

/* Base pointer adjustment for idx->struct table. */
struct TENSIONLC* plc[NUMTENSIONFUNCTIONS];

/* Highflash command CAN id table lookup mapping. */
const uint32_t myfunctype[NUMTENSIONFUNCTIONS] = { \
 FUNCTION_TYPE_TENSION_a,
 FUNCTION_TYPE_TENSION_a2 ,
};
/* **************************************************************************************
 * static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct TENSIONFUNCTION* p); 
 * @brief	: Setup CAN msg with reading
 * @param	: canid = CAN ID
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent
 * @param	: p = pointer to a bunch of things for this function instance
 * ************************************************************************************** */
static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct TENSIONFUNCTION* p)
{
	struct CANRCVBUF can;
	can.id = canid;
	can.dlc = 5;			// Set return msg payload count
	can.cd.uc[0] = status;
	can.cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	can.cd.uc[2] = (*pv >>  8);
	can.cd.uc[3] = (*pv >> 16);
	can.cd.uc[4] = (*pv >> 24);
	can_hub_send(&can, p->phub_tension);// Send CAN msg to 'can_hub'
	p->hbct_ticks = (p->ten_a.hbct * tim3_ten2_rate) / 1000; // Convert ms to timer ticks
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	 // Reset heart-beat time duration each time msg sent
	return;
}

/* **************************************************************************************
 * int tension_a_functionS_init_all(void);
 * @brief	: Initialize all 'tension_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = "r" command can id for this function was not found
 *		: -997 = Add CANHUB failed
 *		: -996 = Adding CAN IDs to hw filter failed
 *		: -995 = "i" command can id not found for this function
 *
 * static int tension_a_functionS_init(int n, struct TENSIONFUNCTION* p );
 * @brief	: Initialize all 'tension_a' functions
 * @param	: n = instance index
 * @param	: p = pointer to things needed for this function
 * @return	: Same as above
 * ************************************************************************************** */
//  Declaration
static int tension_a_functionS_init(int n, struct TENSIONFUNCTION* p );

/* *************************************************** */
/* Init all instances of tension_a_function supported. */
/* *************************************************** */
int tension_a_functionS_init_all(void)
{
	int i;
	int ret;

	/* Do the initialization mess for each instance possible with this program/board */
	for (i = 0; i < NUMTENSIONFUNCTIONS; i++)
	{
		ret = tension_a_functionS_init(i, &ten_f[i]);
		if (ret < 0) return ret;
	}

	/* Setup incoming CAN msg hardware filters for each function instance */
	

	return ret;
}
/* *********************************************************** */
/* Do the initialization mess for a single tension_a function. */
/* *********************************************************** */
static int tension_a_functionS_init(int n, struct TENSIONFUNCTION* p )
{
	int ret;
	int ret2;
	u32 i;

	/* Set pointer to table in highflash.  Base address provided by .ld file */
// TODO routine to find latest updated table in this flash section
	p->pparamflash = (uint32_t*)pparamflash[n];

	/* Copy table entries to struct in sram from high flash. */
	ret = tension_idx_v_struct_copy(&p->ten_a, p->pparamflash);

	/* Set pointers to struct holding the constants for iir filters. */
	for (i = 0; i < NIIR; i++)
	{
		p->iir_filtered[i].pprm = &p->ten_a.iir[i]; // iir filter pointer to filter constants
		p->iir_filtered[i].sw = 0; // Init switch causes iir routine to init upon first call
	}

	/* Setup mask for checking if this function responds to a poll msg. */
	// Combine so the polling doesn't have to do two tests
	p->poll_mask = (p->ten_a.p_pollbit << 8) || (p->ten_a.f_pollbit & 0xff);

	/* First heartbeat time */
	// Convert heartbeat time (ms) to timer ticks (recompute for online update)
	p->hbct_ticks = (p->ten_a.hbct * tim3_ten2_rate) / 1000;
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	

	/* Add this function (tension_a) to the "hub-server" msg distribution. */
	p->phub_tension = can_hub_add_func();	// Set up port/connection to can_hub
	if (p->phub_tension == NULL) return -997;	// Failed

	/* Add CAN IDs to incoming msgs passed by the CAN hardware filter. */ 
	ret2 = can_driver_filter_add_param_tbl(&p->ten_a.code_CAN_filt[0], 0, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed
	
	/* Find command CAN id for this function in table. (__paramflash0a supplied by .ld file) */
	struct FLASHH2* p0a = (struct FLASHH2*)&__paramflash0a;

	/* Check for reasonable size value in table */
	if ((p0a->size == 0) || (p0a->size > NUMCANIDS2)) return -999;

// TODO get the CAN ID for the ldr from low flash and compare to the loader
// CAN id in this table.

	/* Look up "I" (interrogation/incoming) command CAN ID */	
	/* Check if function type code in the table matches our function */
	for (i = 0; i < p0a->size; i++)
	{ 
		if (p0a->slot[i].func == myfunctype[n])
		{
			p->pcanid_cmd_func_i = &p0a->slot[i].canid; // Save pointer
			break;
		}
	}
	if (i >= p0a->size)
		return -995;
	/* Look up "R" (response) command CAN ID */
	for (i = 0; i < p0a->size; i++)
	{ 
		if (p0a->slot[i].func == (myfunctype[n] + CMD_IR_OFFSET))
		{
			p->pcanid_cmd_func_r = &p0a->slot[i].canid; // Save pointer
			return ret;
		}
	}

	return -998;	// Argh! Table size reasonable, but didn't find it.
}
/* **************************************************************************************
 * static float tension_a_scalereading(void);
 * @brief	: Apply offset, scale and corrections to tension_a, last reading
 * return	: float with value
 * ************************************************************************************** */
#ifdef USECICCALIBRATIONSEQUENCE
static float tension_a_scalereading(struct TENSIONFUNCTION* p)
{
	int ntmp1;
	long long lltmp;
	float scaled1;

	lltmp = cic[0][0].llout_save;
	ntmp1 = lltmp/(1<<22);
	ntmp1 += p->ten_a.ad.offset * 4;
	scaled1 = ntmp1;
	scaled1 *= p->ten_a.ad.scale;
	return scaled1;
}
#endif
/* **************************************************************************************
 * double iir_filtered_calib(struct TENSIONFUNCTION* p, uint32_t n); 
 * @brief	: Convert integer IIR filtered value to offset & scaled double
 * @param	: p = pointer to struct with "everything" for this AD7799
 * @return	: offset & scaled
 * ************************************************************************************** */
double iir_filtered_calib(struct TENSIONFUNCTION* p, uint32_t n)
{
	double d;
	double s;
	double dcal;

	/* Scale filter Z^-1 accumulator. */
	int32_t tmp = p->iir_filtered[n].z / p->iir_filtered[n].pprm->scale;
	
	/* Apply offset. */
	tmp += p->ten_a.ad.offset;
	
	/* Convert to double and scale. */
	d = tmp; 		// Convert scaled, filtered int to double
	s = p->ten_a.ad.scale; 	// Convert float to double
	dcal = d * s;		// Calibrated reading
	p->dcalib_lgr = dcal;	// Save calibrated last good reading
	p->fcalib_lgr = p->dcalib_lgr; // Save float version for CAN bus

	/* Set up status byte for reading. */
	p->status_byte = STATUS_TENSION_BIT_NONEW; // Show no new readings
	
	/* New readings flag. */
	if (p->readingsct != p->readingsct_lastpoll)
	{ // Here, there was a new reading since last poll
		p->status_byte = 0;	// Turn (all) flags off
	}

	/* Check reading against limits. */
	if (dcal > p->ten_a.limit_hi)
	{
		p->status_byte |= STATUS_TENSION_BIT_EXCEEDHI;
		return dcal;
	}
	if (dcal < p->ten_a.limit_lo)
	{
		p->status_byte |= STATUS_TENSION_BIT_EXCEEDLO;
		return dcal;
	}
	return dcal;		// Return scaled value
}
/* **************************************************************************************
 * int tension_a_functionS_temperature_poll(void);
 * @brief	: Handler for thermistor-to-temperature conversion, and AD7799 temperature correction.
 * @return	: 0 = no update; 1 = temperature readings updated
 * ************************************************************************************** */
/* *** NOTE ***
   This routine is called from 'main' since the thermistor formulas use functions 
   with doubles.
*/

/* Thermistor conversion things. */
static int adc_temp_flag_prev = 0; // Thermistor readings ready counter

double dscale = (1.0 / (1 << 18));	// ADC filtering scale factor (reciprocal multiply)

void toggle_1led(int led);	// Routine is in 'tension.c'

/* ADC SEQUENCE: ADC of thermistors
0 PA 3 ADC123-IN3	Thermistor on 32 KHz xtal..Thermistor: AD7799 #2
1 PC 0 ADC12 -IN10	Accelerometer X....Thermistor: load-cell #1
2 PC 1 ADC12 -IN11	Accelerometer Y....Thermistor: load-cell #2
3 PC 2 ADC12 -IN12	Accelerometer Z....Thermistor: AD7799 #1
*/
int tension_a_functionS_temperature_poll(void)
{
	int i,j;	// One would expect FORTRAN, but alas it is only C
	double therm[NUMBERADCTHERMISTER_TEN];	// Floating pt of thermistors readings

	/* Check if 'adcsensor_tension' has a new, filtered, reading for us. */
	if (adc_temp_flag[0] != adc_temp_flag_prev)
	{ // Here, a new set of thermistor readings are ready
		j = (0x1 & adc_temp_flag_prev);		// Get current double buffer index
		adc_temp_flag_prev = adc_temp_flag[0];	// Reset flag

		/* Convert filtered long long to double. */
		for (i = 0; i < NUMBERADCTHERMISTER_TEN; i++)
		{	
			therm[i] = adc_readings_cic[j][i]; // Convert to double
			therm[i] = therm[i] * dscale;	   // Scale to 0-4095
		}

		/* Convert ADC readings into uncalibrated degrees Centigrade. */
		/* Then, apply calibration to temperature. */
		j = 0;	// Index input of readings
		for (i = 0; i < NUMTENSIONFUNCTIONS; i++)
		{ 
			ten_f[i].thrm[0] = therm[j];	// Raw thermistor ADC reading
			ten_f[i].degX[0] = temp_calc_param_dbl(therm[j++], &ten_f[i].ten_a.ad.tp[0]); // Apply formula
			ten_f[i].degC[0] = compensation_dbl(&ten_f[i].ten_a.ad.comp_t1[0], 4, ten_f[i].degX[0]);// Adjustment

			ten_f[i].thrm[1] = therm[j];
			ten_f[i].degX[1] = temp_calc_param_dbl(therm[j++], &ten_f[i].ten_a.ad.tp[1]);
			ten_f[i].degC[1] = compensation_dbl(&ten_f[i].ten_a.ad.comp_t2[0], 4, ten_f[i].degX[1]);
		}

		// TODO compute AD7799 temperature offset/scale factors
		// Timing: End of main's loop trigger will pick these up before this recomputes.

//toggle_1led(LEDGREEN2);	// Let the puzzled programmer know the ADC stuff is alive
		return 1;	// Let 'main' know there was an update
	}
	return 0;	// No update
}
/* ######################################################################################
 * int tension_a_functionS_poll(struct CANRCVBUF* pcan, struct TENSIONFUNCTION* p);
 * @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of tension_a function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ###################################################################################### */
//extern unsigned int tim3_ten2_ticks;	// Running count of timer ticks
/* *** NOTE ***
   This routine is called from CAN_poll_loop.c which runs under I2C1_ER_IRQ
*/
int tension_a_functionS_poll(struct CANRCVBUF* pcan, struct TENSIONFUNCTION* p)
{
	int ret = 0;
	union FTINT	// Union for sending float as 4 byte uint32_t
	{
		uint32_t ui;
		float ft;
	}ui; ui.ui = 0; // (initialize to get rid of ui.ft warning)

	/* Check if this instance is used. */
	if ((p->ten_a.useme & 1) != 1) return 0;

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ten2_ticks - (int)p->hb_t) > 0  )	// Time to send heart-beat?
	{ // Here, yes.
		iir_filtered_calib(p, 1);	// Slow (long time constant) filter the reading
		ui.ft = p->fcalib_lgr;		// Float version of calibrated last good reading
		
		/* Send heartbeat and compute next hearbeat time count. */
		//      Args:  CAN id, status of reading, reading pointer instance pointer
		send_can_msg(p->ten_a.cid_heartbeat, p->status_byte, &ui.ui, p); 
		ret = 1;
	}

	/* Check if any incoming msgs. */
	if (pcan == NULL) return ret;

	/* Check for group poll, and send msg if it is for us. */
//$	if (pcan->id == p->ten_a.cid_ten_poll) // Correct ID?
//if (pcan->id == p->ten_a.cid_gps_sync) // ##### TEST #########
if (pcan->id == 0x00400000) // ##### TEST #########
	{ // Here, group poll msg.  Check if poll and function bits are for us
//$		if ( ((pcan->cd.uc[0] & p->ten_a.p_pollbit) != 0) && \
		     ((pcan->cd.uc[1] & p->ten_a.f_pollbit) != 0) )
		{ // Here, yes.  Send our precious msg.
			/* Send tension msg and re-compute next hearbeat time count. */
			//      Args:  CAN id, status of reading, reading pointer instance pointer
			send_can_msg(p->ten_a.cid_ten_msg, p->status_byte, &ui.ui, p); 
			return 1;
		}
	}
	
	/* Check for tension function command. */
	if (pcan->id == *p->pcanid_cmd_func_i)
	{ // Here, we have a command msg for this function instance. 
		cmd_code_dispatch(pcan, p); // Handle and send as necessary
		return 0;	// No msgs passed to other ports
	}
	return ret;
}

