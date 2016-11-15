/******************************************************************************
* File Name          : tension_param.c
* Date First Issued  : 04/08/2015
* Board              : f103
* Description        : Initialize tension function struct with default values &
*                    : define table of index/id versus pointer to struct
*******************************************************************************/
/*
If the parameter loading from the PC is not implemented, then this routine can be
used to "manually" set parameters.
*/

#include "db/can_db.h"
#include "tension_function.h"

/* **************************************************************************************
 * void tension_param_default_init(struct TENSIONLC* pten); 
 * @brief	: Initialize struct with default values
 * ************************************************************************************** */
void tension_param_default_init(struct TENSIONLC* pten)
{
	pten->crc	= ~0;		// CRC for this list.
	pten->version	= 1;		// INCREMENT this number to be greater than the one in flash.

	pten->ad.offset = 0.0;		// Final offset
	pten->ad.scale	= 1.0;		// Final scale

	/* Thermistor #1 parameters */
	pten->ad.tp[0].B 	= 3380.0;	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	pten->ad.tp[0].RS	= 10.0;		// Series resistor, fixed (K ohms)
	pten->ad.tp[0].RO	= 10.0;		// Thermistor room temp resistance (K ohms)
	pten->ad.tp[0].TREF	= 298.0;	// Reference temp for thermistor
	pten->ad.tp[0].offset 	= 0.0;		// Thermistor temp correction (deg C)
	pten->ad.tp[0].scale	= 1.0;		// Thermistor temp correction
	pten->ad.tp[0].poly.offset[0] = 0.0;	// Polynomial coefficients
	pten->ad.tp[0].poly.offset[1] = 1.0;	// Polynomial coefficients
	pten->ad.tp[0].poly.offset[2] = 0.0;	// Polynomial coefficients
	pten->ad.tp[0].poly.offset[3] = 0.0;	// Polynomial coefficients

	/* Thermistor #2 parameters */
	pten->ad.tp[1].B 	= 3380.0;	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	pten->ad.tp[1].RS	= 10.0;		// Series resistor, fixed (K ohms)
	pten->ad.tp[1].RO	= 10.0;		// Thermistor room temp resistance (K ohms)
	pten->ad.tp[1].TREF	= 298.0;	// Reference temp for thermistor
	pten->ad.tp[1].offset 	= 0.0;		// Thermistor temp correction (deg C)
	pten->ad.tp[1].scale	= 1.0;		// Thermistor temp correction
	pten->ad.tp[1].poly.coeff[0] = 0.0;	// Polynomial coefficients
	pten->ad.tp[1].poly.coeff[1] = 1.0;	// Polynomial coefficients
	pten->ad.tp[1].poly.coeff[2] = 0.0;	// Polynomial coefficients
	pten->ad.tp[1].poly.coeff[3] = 0.0;	// Polynomial coefficients

	/* Something for the hapless Op (and others!) */
	pten->c0 = {('T' <<  0),('E' <<  8) ,('N' << 16), ('S' << 24)};
	pten->c1 = {('I' <<  0),('O' <<  8) ,('N' << 16), (' ' << 24)};
	pten->c2 = {('d' <<  0),('e' <<  8) ,('f' << 16), ('a' << 24)};
	pten->c3 = {('u' <<  0),('l' <<  8) ,('t' << 16), ( 0  << 24)};

	pten->hb_ct		= 64;			// Tension: Heart-Beat: Count of time ticks between autonomous msgs
	pten->drum_number	= 0;			// Tension: Drum system number for this function instance
	pten->drum_poll_bit 	= 0x1;			// Tension: Drum system poll 1st byte bit for function instance
	pten->drum_function_bit	= 0x1;			// Tension: Drum system poll 2nd byte bit for this type of function

	/* Use the CANID_NAME from the CANID table in the following, as these are the default CAN IDs. */
	pten->canid_drum_poll	= CANID_TIME_MSG;	// MC: Time msg for group polling
	pten->canid_measurement	= CANID_TENSION_0; 	// default canid for sending polled tension 
	pten->canid_time_sync	= CANID_TIMESYNC;	// Time: CANID Command GPS

	/* Some test cases. */
	pten->testdbl = -3.14159265358979323;
	pten->testull = 0x0123456789ABCDEF;
	pten->testsll = -1000;

	return;
}
/* ***********************************************************************************************************************************
 * tension_param_table = ptentbl
 * @brief	: Table to relate index/id number with pointer into SRAM struct for storing 4 byte value
 * *********************************************************************************************************************************** */
const struct PARAM_TABLE ptentbl[PARAM_LIST_CT_TENSION] = { \
{&pten->crc		, TENSION_LIST_CRC	 		},   /* Tension: CRC for for this list */\
{&pten->ver		, TENSION_LIST_VERSION   		},   /* Tension: Version number for Tension List */ \
{pten->ad.offset 	, TENSION_AD7799_1_OFFSET		},   /* Tension: AD7799 offset */ \
{pten->ad.scale		, TENSION_AD7799_1_SCALE 		},   /* Tension: AD7799 #1 Scale (convert to kgf) */ \

/* Thermistor #1 parameters */
{&pten->ad.tp[0].B 		, TENSION_THERM1_CONST_B    	},   /* Tension: Thermistor1 param: constant B */ \
{&pten->ad.tp[0].RS		, TENSION_THERM1_R_SERIES   	},   /* Tension: Thermistor1 param: Series resistor, fixed (K ohms) */ \
{&pten->ad.tp[0].RO		, TENSION_THERM1_R_ROOMTMP  	},   /* Tension: Thermistor1 param: Thermistor room temp resistance (K ohms) */ \
{&pten->ad.tp[0].TREF		, TENSION_THERM1_REF_TEMP   	},   /* Tension: Thermistor1 param: Reference temp for thermistor */ \
{&pten->ad.tp[0].offse; 	, TENSION_THERM1_TEMP_OFFSET	},   /* Tension: Thermistor1 param: Thermistor temp offset correction (deg C) */ \
{&pten->ad.tp[0].scale;		, TENSION_THERM1_TEMP_SCALE 	},   /* Tension: Thermistor1 param: Thermistor temp scale correction */ \
{&pten->ad.tp[0].poly.coeff[0]	, TENSION_THERM1_LC_COEF_0  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */ \
{&pten->ad.tp[0].poly.coeff[1]	, TENSION_THERM1_LC_COEF_1  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */ \
{&pten->ad.tp[0].poly.coeff[2]	, TENSION_THERM1_LC_COEF_2  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */ \
{&pten->ad.tp[0].poly.coeff[3]	, TENSION_THERM1_LC_COEF_3  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */ \

/* Thermistor #2 parameters */
{&pten->ad.tp[1].B 		, TENSION_THERM2_CONST_B    	},   /* Tension: Thermistor2 param: constant B */ \
{&pten->ad.tp[1].RS		, TENSION_THERM2_R_SERIES   	},   /* Tension: Thermistor2 param: Series resistor, fixed (K ohms) */ \
{&pten->ad.tp[1].RO		, TENSION_THERM2_R_ROOMTMP  	},   /* Tension: Thermistor2 param: Thermistor room temp resistance (K ohms) */ \
{&pten->ad.tp[1].TREF		, TENSION_THERM2_REF_TEMP   	},   /* Tension: Thermistor2 param: Reference temp for thermistor */ \
{&pten->ad.tp[1].offset 	, TENSION_THERM2_TEMP_OFFSET	},   /* Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */ \
{&pten->ad.tp[1].scale		, TENSION_THERM2_TEMP_SCALE 	},   /* Tension: Thermistor2 param: Thermistor temp scale correction */ \
{&pten->ad.tp[1].poly.offset[0]	, TENSION_THERM2_LC_COEF_0  	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */ \
{&pten->ad.tp[1].poly.offset[1] , TENSION_THERM2_LC_COEF_1  	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */ \
{&pten->ad.tp[1].poly.offset[2] , TENSION_THERM2_LC_COEF_2  	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */ \
{&pten->ad.tp[1].poly.offset[3] , TENSION_THERM2_LC_COEF_3 	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */ \

/* Something text for identifying... */
{&pten->c0 	, TENSION_ASCII_0, 			   	}, 	/* ASCII identifier for the Wizard Programmer, */ \
{&pten->c1 	, TENSION_ASCII_1,   				},	/*   hapless Op, and awed Onlooker. */ \
{&pten->c2	, TENSION_ASCII_2,   				}, 	\
{&pten->c3 	, TENSION_ASCII_3,   				},	\

{&pten->hb_ct			, TENSION_HEARTBEAT_CT	    	}, 	/* Heart-beat: Number of ticks between msgs */\
{&pten->drum_number		, TENSION_DRUM_NUMBER		},	/* Drum number of this function */\
{&pten->drum_poll_bit		, TENSION_DRUM_POLL_BIT		},	/* Tension: Drum system poll 1st byte bit for function instance */\
{&pten->drum_function_bit	, POLL_FUNC_BIT_TENSION		},	/* Tension: Drum system poll 2nd byte bit for this type of function */\

/* Use the parameter PARAM_NAME from the PARAM_LIST table, as these relate the index/id to the pointer, and not the value. */\
{&pten->canid_drum_poll		, TENSION_CANPRM_GROUP_POLL	},	/* Tension: CANID: msg that polls group (drum) functions  */\
{&pten->canid_measurement 	, TENSION_CANPRM_TENSION	},	/* Tension: CANID: can msg tension */\ 
{&pten->canid_time_sync   	, TENSION_CANPRM_TIMESYNC	},	/* Tension: CANID: msg for time syncing local clocks */\

/* Test multi-chunk transfers. */
{&(pten->testdbl + 0)		, TENSION_TEST_DOUBLE_L		},	/* Tension: Test: double low  order 4 bytes */\
{&(pten->testdbl + 1)		, TENSION_TEST_DOUBLE_H		},	/* Tension: Test: double high order 4 bytes */\
{&(pten->testull + 0)		, TENSION_TEST_U64_L		},	/* Tension: Test: unsigned long long, low  order 4 bytes */\
{&(pten->testull + 1)		, TENSION_TEST_U64_H		},	/* Tension: Test: unsigned long long, high order 4 bytes */\
{&(pten->testsll + 0)		, TENSION_TEST_S64_L		},	/* Tension: Test:   signed long long, low  order 4 bytes */\
{&(pten->testsll + 1)		, TENSION_TEST_S64_H		},	/* Tension: Test:   signed long long, high order 4 bytes */\

};


