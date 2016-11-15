/******************************************************************************
* File Name          : cable_angle_readings.C
* Date First Issued  : 04/13/2015
* Board              : f103
* Description        : Initialize cable_angle function struct with default values
*******************************************************************************/
/*
This routine sends back the "reading" specified in the command.
Received--
payload byte 1 = command code
payload byte 2 = index/identification
Send--
Same CAN ID and 1st two bytes, with payload bytes 3 -6 holding the
  the four byte value.

Return 0 if the index/identification and payload of zero is not in the 'case' statements.

NOTE: This routine is essentially identical to 'tension_readings.c' as it be (most likely)
sharing the same CAN driver.  The readings for the error counts are the same for both.
*/

#include <stdint.h>
#include "db/can_db.h"
#include "cable_angle__function.h"
#include "canwinch_pod_common_systick2048.h"

/* **************************************************************************************
 * static send_can(struct CANRCVBUF* pcan, uint32_t v); 
 * @brief	: Setup DLC and payload for sending a current reading
 * @param	: Pointer to incoming CAN msg that has "readings" command
 * @param	: v = 4 byte value to be sent
 * ************************************************************************************** */
static send_can(struct CANRCVBUF* pcan, uint32_t v)
{
	/* The values may be be changing under interrupt, so get a local copy before pulling out bytes.*/
	uint32_t v = *p;

	pcan->dlc = 6;			// Set return msg payload count
	pcan->cd.uc[3] = (v >>  0);	// Add 4 byte value to payload
	pcan->cd.uc[4] = (v >>  8);
	pcan->cd.uc[5] = (v >> 16);
	pcan->cd.uc[6] = (v >> 24);
	canwinch_pod_common_systick2048.h(pcan);	// Send CAN msg
	return;
}
/* **************************************************************************************
 * void cable_angle__readings_respond(struct CANRCVBUF* pcan); 
 * @brief	: Send a readings (actually send a memory location that holds a reading)
 * @param	: pcan = pointer to incoming CAN msg
 * ************************************************************************************** */
void cable_angle__readings_respond(struct CANRCVBUF* pcan)
{
	if (pcan->cd.uc[0] != CMD_GET_READING) return;	// Return if not "Send a reading code specified in byte [1]"

	switch(pcan->cd.uc[1]);	// Switch on index/id number
	{
	case CABLE_ANGLE_READ_ADC_THERM1:  	// 1, 'TYP_U32' ,'%u', 	'CABLE_ANGLE', 'Cable Angle: READING: Filtered ADC for Thermistor 1');
		send_can(pcan, therm1_adc); break;

	case CABLE_ANGLE_READ_ADC_THERM2:  	// 2, 'TYP_U32' ,'%u',	'CABLE_ANGLE', 'Cable Angle: READING: Filtered ADC for Thermistor 2');
		send_can(pcan, therm2_adc); break;

	case CABLE_ANGLE_READ_CALIB_THERM1:	// 3, 'TYP_FLT' ,'%0.2f','CABLE_ANGLE', 'Cable Angle: READING: Calibrated temperaure for Thermistor 1');
		send_can(pcan, (unit32_t)therm1_calib); break;

	case CABLE_ANGLE_READ_CALIB_THERM2:	// 4, 'TYP_FLT' ,'%0.2f','CABLE_ANGLE', 'Cable Angle: READING: Calibrated temperaure for Thermistor 2');
		send_can(pcan, (unit32_t)therm2_calib); break;

	case CABLE_ANGLE_READ_AD7799_RAW:	// 5, 'TYP_S32' ,'%u', 	'CABLE_ANGLE', 'Cable Angle: READING: AD7799 #1 raw reading');
		send_can(pcan, (unit32_t)ad7799_1_raw); break;

	case CABLE_ANGLE_READ_AD7799_CALIB:	// 6, 'TYP_FLT' ,'0.3f', 'CABLE_ANGLE', 'Cable Angle: READING: AD7799 #1 calibrated reading');
		send_can(pcan, (unit32_t)ad7799_1_calib); break;

	case CABLE_ANGLE_READ_CANMSG_OVFLO:	// 7, 'TYP_U32' ,'%u', 	'CABLE_ANGLE', 'Cable Angle: can_msgovrflow: Count of CAN msg buffer overflow');
		send_can(pcan, can_msgovrflow); break;

	case CABLE_ANGLE_READ_CAN_FIFO1_CT:	// 8, 'TYP_U32' ,'%u', 	'CABLE_ANGLE', 'Cable Angle: error_fifo1ctr: Count of CAN too many time sync msgs in one 1/64th systick interval');
		send_can(pcan, error_fifo1ctr); break;

	case CABLE_ANGLE_READ_CAN_TXERR_CT:	// 9, 'TYP_U32' ,'%u', 	'CABLE_ANGLE', 'Cable Angle: can_txerr     : Count of CAN TX error status');
		send_can(pcan, can_txerr); break;

	case CABLE_ANGLE_READ_CAN_RX0ERR_CT	//10, 'TYP_U32' ,'%u', 	'CABLE_ANGLE', 'Cable Angle: can_rx0err    : Count of CAN FIFO 0 overruns');
		send_can(pcan, can_rx0err); break;

	case CABLE_ANGLE_READ_CAN_RX1ERR_CT	//11, 'TYP_U32' ,'%u', 	'CABLE_ANGLE', 'Cable Angle: can_rx1err    : Count of CAN FIFO 1 overruns');
		send_can(pcan, can_rx1err); break;
		break;

	default:
		pcan->cd.uc[1]= 0;	// Send error code: idex/idx not in 'case'
		send_can(pcan, 0); break;
		break;
	}
	return;
}
