/******************************************************************************
* File Name          : cable_angle_param.c
* Date First Issued  : 04/12/2015
* Board              : f103
* Description        : Initialize cable angle function struct with default values &
*                    : define table of index/id versus pointer to struct
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/can_db.h"
#include "cable_angle_function.h"

/* **************************************************************************************
 * void cable_angle_param_default_init(struct TENSIONLC* pcbl); 
 * @brief	: Initialize struct with default values
 * ************************************************************************************** */
void cable_angle_param_default_init(struct TENSIONLC* pcbl)
{
	pcbl->crc	= ~0;		// CRC for this list.
	pcbl->version	= 1;		// INCREMENT this number to be greater than the one in flash.

	pcbl->ad.offset = 0.0;		// Final offset
	pcbl->ad.scale	= 1.0;		// Final scale

	/* Thermistor #1 parameters */
	pcbl->ad.tp[0].B 	= 3380.0;	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	pcbl->ad.tp[0].RS	= 10.0;		// Series resistor, fixed (K ohms)
	pcbl->ad.tp[0].RO	= 10.0;		// Thermistor room temp resistance (K ohms)
	pcbl->ad.tp[0].TREF	= 298.0;	// Reference temp for thermistor
	pcbl->ad.tp[0].offset 	= 0.0;		// Thermistor temp correction (deg C)
	pcbl->ad.tp[0].scale	= 1.0;		// Thermistor temp correction
	pcbl->ad.tp[0].poly.offset[0] = 0.0;	// Polynomial coefficients
	pcbl->ad.tp[0].poly.offset[1] = 1.0;	// Polynomial coefficients
	pcbl->ad.tp[0].poly.offset[2] = 0.0;	// Polynomial coefficients
	pcbl->ad.tp[0].poly.offset[3] = 0.0;	// Polynomial coefficients

	/* Thermistor #2 parameters */
	pcbl->ad.tp[1].B 	= 3380.0;	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	pcbl->ad.tp[1].RS	= 10.0;		// Series resistor, fixed (K ohms)
	pcbl->ad.tp[1].RO	= 10.0;		// Thermistor room temp resistance (K ohms)
	pcbl->ad.tp[1].TREF	= 298.0;	// Reference temp for thermistor
	pcbl->ad.tp[1].offset 	= 0.0;		// Thermistor temp correction (deg C)
	pcbl->ad.tp[1].scale	= 1.0;		// Thermistor temp correction
	pcbl->ad.tp[1].poly.offset[0] = 0.0;	// Polynomial coefficients
	pcbl->ad.tp[1].poly.offset[1] = 1.0;	// Polynomial coefficients
	pcbl->ad.tp[1].poly.offset[2] = 0.0;	// Polynomial coefficients
	pcbl->ad.tp[1].poly.offset[3] = 0.0;	// Polynomial coefficients

	/* Something for the hapless Op (and others!) */
	pcbl->c0 = {('C' <<  0),('A' <<  8) ,('B' << 16), ('L' << 24)};
	pcbl->c1 = {('E' <<  0),(' ' <<  8) ,('A' << 16), ('N' << 24)};
	pcbl->c2 = {('G' <<  0),('L' <<  8) ,('E' << 16), ('_' << 24)};
	pcbl->c3 = {('0' <<  0),( 0  <<  8) ,( 0  << 16), ( 0  << 24)};

	pcbl->hb_ct		= 64;			// Cable Angle: Heart-Beat: Count of time ticks between autonomous msgs
	pcbl->drum_number	= 0;			// Cable Angle: Drum system number for this function instance
	pcbl->drum_poll_bit 	= 0x1;			// Cable Angle: Drum system poll 1st byte bit for function instance
	pcbl->drum_function_bit	= 0x2;			// Cable Angle: Drum system poll 2nd byte bit for this type of function

	/* Use the CANID_NAME from the CANID table in the following, as these are the default CAN IDs. */
	pcbl->canid_drum_poll	= CANID_TIME_MSG;	// MC: Time msg for group polling
	pcbl->canid_measurement = CANID_CABLE_ANGLE_0; 	// default canid for sending polled tension 
	pcbl->canid_time_sync	= CANID_TIMESYNC;	// Time: CANID Command GPS

	pcbl->msg_rate_ct = 2;				// Rate count: Number of tension readings between cable angle msgs
	pcbl->alarm_repeat_ct = 3;			// Number of times alarm msg is repeated

	pcbl->cable_angle_coeff[0] = 0.0;		// CABLE_ANGLE_CALIB_COEF_0 Cable Angle: Cable angle polynomial coefficient 0
	pcbl->cable_angle_coeff[1] = 1.0;		// CABLE_ANGLE_CALIB_COEF_0 Cable Angle: Cable angle polynomial coefficient 1
	pcbl->cable_angle_coeff[2] = 0.0;		// CABLE_ANGLE_CALIB_COEF_0 Cable Angle: Cable angle polynomial coefficient 2
	pcbl->cable_angle_coeff[3] = 0.0;		// CABLE_ANGLE_CALIB_COEF_0 Cable Angle: Cable angle polynomial coefficient 3

	return;
}
/* ***********************************************************************************************************************************
 * cable_angle_param_table = pcbltbl
 * @brief	: Table to relate index/id number with pointer into SRAM struct for storing 4 byte value
 * *********************************************************************************************************************************** */
const struct PARAM_TABLE pcbltbl[PARAM_LIST_CT_CABLE_ANGLE] = { \
{&pcbl->crc		, CABLE_ANGLE_LIST_CRC	  		},	/* Cable Angle: CRC for for this list */\
{&pcbl->ver		, CABLE_ANGLE_LIST_VERSION   		},	/* Cable Angle: Version number for Tension List */ \
{pcbl->ad.offset 	, CABLE_ANGLE_AD7799_1_OFFSET		}, 	/* Cable Angle: AD7799 offset */ \
{pcbl->ad.scale		, CABLE_ANGLE_AD7799_1_SCALE 		},	/* Cable Angle: AD7799 #1 Scale (convert to kgf) */ \

/* Thermistor #1 parameters */
{&pcbl->ad.tp[0].B 		, CABLE_ANGLE_THERM1_CONST_B    	},   /* Cable Angle: Thermistor1 param: constant B */ \
{&pcbl->ad.tp[0].RS		, CABLE_ANGLE_THERM1_R_SERIES   	},   /* Cable Angle: Thermistor1 param: Series resistor, fixed (K ohms) */ \
{&pcbl->ad.tp[0].RO		, CABLE_ANGLE_THERM1_R_ROOMTMP  	},   /* Cable Angle: Thermistor1 param: Thermistor room temp resistance (K ohms) */ \
{&pcbl->ad.tp[0].TREF		, CABLE_ANGLE_THERM1_REF_TEMP   	},   /* Cable Angle: Thermistor1 param: Reference temp for thermistor */ \
{&pcbl->ad.tp[0].offse; 	, CABLE_ANGLE_THERM1_TEMP_OFFSET	},   /* Cable Angle: Thermistor1 param: Thermistor temp offset correction (deg C) */ \
{&pcbl->ad.tp[0].scale;		, CABLE_ANGLE_THERM1_TEMP_SCALE 	},   /* Cable Angle: Thermistor1 param: Thermistor temp scale correction */ \
{&pcbl->ad.tp[0].poly.offset[0] , CABLE_ANGLE_THERM1_LC_COEF_0  	},   /* Cable Angle: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */ \
{&pcbl->ad.tp[0].poly.offset[1] , CABLE_ANGLE_THERM1_LC_COEF_1  	},   /* Cable Angle: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */ \
{&pcbl->ad.tp[0].poly.offset[2] , CABLE_ANGLE_THERM1_LC_COEF_2  	},   /* Cable Angle: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */ \
{&pcbl->ad.tp[0].poly.offset[3] , CABLE_ANGLE_THERM1_LC_COEF_3  	},   /* Cable Angle: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */ \

/* Thermistor #2 parameters */
{&pcbl->ad.tp[1].B 		, CABLE_ANGLE_THERM2_CONST_B    	},   /* Cable Angle: Thermistor2 param: constant B */ \
{&pcbl->ad.tp[1].RS		, CABLE_ANGLE_THERM2_R_SERIES   	},   /* Cable Angle: Thermistor2 param: Series resistor, fixed (K ohms) */ \
{&pcbl->ad.tp[1].RO		, CABLE_ANGLE_THERM2_R_ROOMTMP  	},   /* Cable Angle: Thermistor2 param: Thermistor room temp resistance (K ohms) */ \
{&pcbl->ad.tp[1].TREF		, CABLE_ANGLE_THERM2_REF_TEMP   	},   /* Cable Angle: Thermistor2 param: Reference temp for thermistor */ \
{&pcbl->ad.tp[1].offset 	, CABLE_ANGLE_THERM2_TEMP_OFFSET	},   /* Cable Angle: Thermistor2 param: Thermistor temp offset correction (deg C) */ \
{&pcbl->ad.tp[1].scale		, CABLE_ANGLE_THERM2_TEMP_SCALE 	},   /* Cable Angle: Thermistor2 param: Thermistor temp scale correction */ \
{&pcbl->ad.tp[1].poly.offset[0]	, CABLE_ANGLE_THERM2_LC_COEF_0  	},   /* Cable Angle: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */ \
{&pcbl->ad.tp[1].poly.offset[1] , CABLE_ANGLE_THERM2_LC_COEF_1  	},   /* Cable Angle: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */ \
{&pcbl->ad.tp[1].poly.offset[2] , CABLE_ANGLE_THERM2_LC_COEF_2  	},   /* Cable Angle: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */ \
{&pcbl->ad.tp[1].poly.offset[3] , CABLE_ANGLE_THERM2_LC_COEF_3	 	},   /* Cable Angle: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */ \

/* Something text for identifying... */
{&pcbl->c0 , CABLE_ANGLE_ASCII_0, 				},  	/* ASCII identifier for the Wizard Programmer, */ \
{&pcbl->c1 , CABLE_ANGLE_ASCII_1, 				},	/*   hapless Op, and awed Onlooker. */ \
{&pcbl->c2,  CABLE_ANGLE_ASCII_2,				}, 	\
{&pcbl->c3 , CABLE_ANGLE_ASCII_3, 				},	\

{&pcbl->hb_ct			, CABLE_ANGLE_HEARTBEAT_CT	},	/* Heart-beat: Number of ticks between msgs */\
{&pcbl->drum_number		, CABLE_ANGLE_DRUM_NUMBER	},	/* Drum number of this function */\
{&pcbl->drum_poll_bit		, CABLE_ANGLE_DRUM_POLL_BIT	},	/* Cable Angle: Drum system poll 1st byte bit for function instance */\
{&pcbl->drum_function_bit	, POLL_FUNC_BIT_CABLE_ANGLE	},	/* Cable Angle: Drum system poll 2nd byte bit for this type of function */\

/* Use the parameter PARAM_NAME from the PARAM_LIST table, as these relate the index/id to the pointer, and not the value. */\
{&pcbl->canid_drum_poll		, CABLE_ANGLE_CANPRM_GROUP_POLL	},	/* Cable Angle:  CANID: msg that polls group (drum) functions  */\
{&pcbl->canid_measurement 	, CABLE_ANGLE_CANPRM_TENSION	},	/* Cable Angle:  CANID: can msg tension */\ 
{&pcbl->canid_time_sync   	, CABLE_ANGLE_CANPRM_TIMESYNC	},	/* Cable Angle:  CANID: msg for time syncing local clocks */\

{&pcbl->msg_rate_ct 		,CABLE_ANGLE_MIN_TENSION	},	/* Cable Angle: Minimum tension required (units to match) */\
{&pcbl->alarm_repeat_ct 	,CABLE_ANGLE_RATE_CT		},	/* Cable Angle: Rate count: Number of tension readings between cable angle msgs */\
{&pcbl->alarm_repeat_ct 	,CABLE_ANGLE_ALARM_REPEAT	},	/* Cable Angle: Number of times alarm msg is repeated */\
{&pcbl->cable_angle_coeff[0] 	,CABLE_ANGLE_CALIB_COEF_0	},	/* Cable Angle: Cable angle polynomial coefficient 0 */\
{&pcbl->cable_angle_coeff[1] 	,CABLE_ANGLE_CALIB_COEF_1	},	/* Cable Angle: Cable angle polynomial coefficient 1 */\
{&pcbl->cable_angle_coeff[2] 	,CABLE_ANGLE_CALIB_COEF_2	},	/* Cable Angle: Cable angle polynomial coefficient 2 */\
{&pcbl->cable_angle_coeff[3] 	,CABLE_ANGLE_CALIB_COEF_3	},	/* Cable Angle: Cable angle polynomial coefficient 3 */\

};


