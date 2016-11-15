/******************************************************************************
* File Name          : tension_a_functionS.h
* Date First Issued  : 05/29/2016
* Board              : f103
* Description        : Tension function_a for both AD7799s
*******************************************************************************/

#ifndef __TENSION_A_FUNCTIONS
#define __TENSION_A_FUNCTIONS

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "tension_idx_v_struct.h"
#include "ad7799_filter_ten2.h"
#include "iir_filter_l.h"
#include "queue_dbl.h"

#define CMD_IR_OFFSET 1000	// Command CAN function ID table offset for "R" CAN ID

/* Accumulating average (useful for determining offset). */
struct ACCUMAVE
{
	int64_t sum;		// Sum readings
	int32_t n;		// Keep track of number of readings
	int32_t a;		// Computed average
	uint8_t run;		// Switch: 0 = skip; not zero = build average
	uint8_t run_prev;	// Previous state of run
};


/* This is the working struct for *each* ADD7799. */
struct TENSIONFUNCTION
{
	/* The following is the sram copy of the fixed (upper flash) parameters */
	struct TENSIONLC ten_a;		// Flash table copied to sram struct
	/* The following are working/computed values */
	struct IIRFILTERL iir_filtered[NIIR]; // IIR filters for one AD7799
	struct CANHUB* phub_tension;	// Pointer: CAN hub buffer
	struct ACCUMAVE ave;		// Accumulating average
	double thrm[2];			// Filtered reading converted to double
	double degX[2];			// Uncalibrated temperature for each thermistor
	double degC[2];			// Calibrated temperature for each thermistor
	double ten_iircal[NIIR];	// Tension: filtered and calibrated
	void* ptension_idx_struct;	// Pointer to table of pointers for idx->struct 
	void* pparamflash;		// Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_func_i;	// Pointer into high flash for command can id (incoming)
	uint32_t* pcanid_cmd_func_r;	// Pointer into high flash for command can id (response)
	uint32_t hb_t;			// tim3 tick counter for next heart-beat CAN msg
	uint32_t hbct_ticks;		// ten_a.hbct (ms) converted to timer ticks
	int32_t lgr;			// Last Good Reading (of AD7799)
	int32_t cicraw; 		// CIC filtered w/o any adjustments for one AD7799
	long long cicave;		// Averaged cic readings for nice display
	int32_t offset_reg;		// Last reading of AD7799 offset register
	int32_t offset_reg_filt;	// Last filtered AD7799 offset register
	int32_t offset_reg_rdbk;	// Last filtered AD7799 offset register set read-back
	int32_t fullscale_reg;		// Last reading of AD7799 fullscale register
	uint16_t poll_mask; 		// Mask for first two bytes of a poll msg 		(necessary?)
	uint32_t readingsct;		// Running count of readings (conversions completed)
	uint32_t readingsct_lastpoll;	// Reading count the last time a poll msg sent
	struct IIRFILTERL iir_offset; 	// IIR filter for offsets
	uint32_t offset_ct;		// Running ct of offset updates
	uint32_t offset_ct_prev;	// Previous ct of offset updates
	int16_t zero_flag;		// 1 = zero-calibration operation competed
	struct IIRFILTERL iir_z_recal_w;// Working zero recal filter
	uint32_t z_recal_next;		// Conversion count for next zero recalibration
	uint8_t status_byte;		// Reading status byte
	double	dcalib_lgr;		// (double) Calibrated, last good reading
	float	fcalib_lgr;		// (float) calibrated last good reading
};

/* **************************************************************************************/
int tension_a_functionS_init_all(void);
/* @brief	: Initialize all 'tension_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = command can id for this function was not found
 * ************************************************************************************** */
int tension_a_functionS_poll(struct CANRCVBUF* pcan, struct TENSIONFUNCTION* p);
/* @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of tension_a function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ************************************************************************************** */
int tension_a_functionS_temperature_poll(void);
/* @brief	: Handler for thermistor-to-temperature conversion, and AD7799 temperature correction.
 * @return	: 0 = no update; 1 = temperature readings updated
 * ************************************************************************************** */


/* Holds parameters and associated computed values and readings for each instance. */
extern struct TENSIONFUNCTION ten_f[NUMTENSIONFUNCTIONS];

#endif 

