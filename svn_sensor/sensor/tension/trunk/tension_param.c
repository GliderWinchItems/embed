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

#include "db/gen_db.h"
#include "tension_function.h"
#include "tension_idx_v_struct.h"

struct PARAM_TENSIONLC
{
	void    *p;	// Pointer to parameter in struct
	u32	id;	// ID number of parameter
};

/* **************************************************************************************
 * void tension_param_default_init(struct TENSIONLC* pten); 
 * @brief	: Initialize struct with default values 
 * ************************************************************************************** */
void tension_param_default_init(struct TENSIONLC* pten)
{
	pten->size	= PARAM_LIST_CT_TENSION_a;	// Number of items in list
	pten->crc	= ~0;		// CRC for this list.
	pten->version	= 1;		// INCREMENT this number to be greater than the one in flash.

	pten->ad.offset = 0.0;		// Final offset
	pten->ad.scale	= 1.0;		// Final scale

	/* Thermistor #1 parameters */
	pten->ad.tp[0].B 	= 3380.0;	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	pten->ad.tp[0].RS	= 10.0;		// Series resistor, fixed (K ohms)
	pten->ad.tp[0].R0	= 10.0;		// Thermistor room temp resistance (K ohms)
	pten->ad.tp[0].TREF	= 298.0;	// Reference temp for thermistor
	pten->ad.tp[0].offset 	= 0.0;		// Thermistor temp correction (deg C)
	pten->ad.tp[0].scale	= 1.0;		// Thermistor temp correction


	/* Thermistor #2 parameters */
	pten->ad.tp[1].B 	= 3380.0;	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	pten->ad.tp[1].RS	= 10.0;		// Series resistor, fixed (K ohms)
	pten->ad.tp[1].R0	= 10.0;		// Thermistor room temp resistance (K ohms)
	pten->ad.tp[1].TREF	= 298.0;	// Reference temp for thermistor
	pten->ad.tp[1].offset 	= 0.0;		// Thermistor temp correction (deg C)
	pten->ad.tp[1].scale	= 1.0;		// Thermistor temp correction


    pten->ad.comp_t1[0] = 0;	/* 17 TENSION_THERM1_LC_COEF_0,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
    pten->ad.comp_t1[1] = 1;	/* 18 TENSION_THERM1_LC_COEF_1,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
    pten->ad.comp_t1[2] = 0;	/* 19 TENSION_THERM1_LC_COEF_2,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
    pten->ad.comp_t1[3] = 0;	/* 20 TENSION_THERM1_LC_COEF_3,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
    pten->ad.comp_t2[0] = 0;	/* 21 TENSION_THERM2_LC_COEF_0,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
    pten->ad.comp_t2[1] = 1;	/* 22 TENSION_THERM2_LC_COEF_1,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
    pten->ad.comp_t2[2] = 0;	/* 23 TENSION_THERM2_LC_COEF_2,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
    pten->ad.comp_t2[3] = 0;	/* 24 TENSION_THERM2_LC_COEF_3,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
    pten->hbct		= 16;	/* 25 TENSION_HEARTBEAT_CT	Tension: hbct: Heart-Beat Count of time ticks between autonomous msgs */
    pten->drum		= 1;	/* 26 TENSION_DRUM_NUMBER	Tension: drum: Drum system number for this function instance */
    pten->f_pollbit	= 0x1;	/* 27 TENSION_DRUM_FUNCTION_BIT	Tension: bit: f_pollbit: Drum system poll 1st byte bit for function instance */
    pten->p_pollbit	= 0x1;	/* 28 TENSION_DRUM_POLL_BIT	Tension: bit: p_pollbit: Drum system poll 2nd byte bit for this type of function */
    pten->cid_ten_msg	=		/* 29 TENSION_DRUM_POLL_BIT	Tension: CANID: cid_ten_msg:  canid msg Tension */
    pten->cid_ten_poll	= 0x1;	/* 30 TENSION_DRUM_FUNCTION_BIT	Tension: CANID: cid_ten_msg:  canid MC: Time msg/Group polling */
    pten->cid_gps_sync	= 0;	/* 31 TENSION_TIMESYNC		Tension: CANID: cid_gps_sync: canid time: GPS time sync distribution */
//    NULL;			/* 00 End of list: (since valid pointers must not be NULL)  */

	return;
}

/* ***********************************************************************************************************************************
 * tension_param_table = ptentbl
 * @brief	: Table to relate id number with pointer into SRAM struct for storing 4 byte value. 
 * *********************************************************************************************************************************** */
const struct PARAM_TENSIONLC ptentbl[PARAM_LIST_CT_TENSION_a] = { \
{&ten_a.size		, PARAM_LIST_CT_TENSION_a		},
{&ten_a.crc		, TENSION_a_LIST_CRC	 		},   /* Tension: CRC for for this list */
{&ten_a.version		, TENSION_a_LIST_VERSION   		},   /* Tension: Version number for Tension List */ 
{&ten_a.ad.offset 	, TENSION_a_AD7799_1_OFFSET		},   /* Tension: AD7799 offset */ 
{&ten_a.ad.scale	, TENSION_a_AD7799_1_SCALE 		},   /* Tension: AD7799 #1 Scale (convert to kgf) */ 

/* Thermistor #1 parameters */
{&ten_a.ad.tp[0].B 		, TENSION_a_THERM1_CONST_B    	},   /* Tension: Thermistor1 param: constant B */ 
{&ten_a.ad.tp[0].RS		, TENSION_a_THERM1_R_SERIES   	},   /* Tension: Thermistor1 param: Series resistor, fixed (K ohms) */ 
{&ten_a.ad.tp[0].R0		, TENSION_a_THERM1_R_ROOMTMP  	},   /* Tension: Thermistor1 param: Thermistor room temp resistance (K ohms) */ 
{&ten_a.ad.tp[0].TREF		, TENSION_a_THERM1_REF_TEMP   	},   /* Tension: Thermistor1 param: Reference temp for thermistor */ 
{&ten_a.ad.tp[0].offset 	, TENSION_a_THERM1_TEMP_OFFSET	},   /* Tension: Thermistor1 param: Thermistor temp offset correction (deg C) */ 
{&ten_a.ad.tp[0].scale		, TENSION_a_THERM1_TEMP_SCALE 	},   /* Tension: Thermistor1 param: Thermistor temp scale correction */ 

/* Thermistor #2 parameters */
{&ten_a.ad.tp[1].B 		, TENSION_a_THERM2_CONST_B    	},   /* Tension: Thermistor2 param: constant B */ 
{&ten_a.ad.tp[1].RS		, TENSION_a_THERM2_R_SERIES   	},   /* Tension: Thermistor2 param: Series resistor, fixed (K ohms) */ 
{&ten_a.ad.tp[1].R0		, TENSION_a_THERM2_R_ROOMTMP  	},   /* Tension: Thermistor2 param: Thermistor room temp resistance (K ohms) */ 
{&ten_a.ad.tp[1].TREF		, TENSION_a_THERM2_REF_TEMP   	},   /* Tension: Thermistor2 param: Reference temp for thermistor */ 
{&ten_a.ad.tp[1].offset 	, TENSION_a_THERM2_TEMP_OFFSET	},   /* Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */ 
{&ten_a.ad.tp[1].scale		, TENSION_a_THERM2_TEMP_SCALE 	},   /* Tension: Thermistor2 param: Thermistor temp scale correction */ 

{&ten_a.ad.comp_t1[0]		, TENSION_a_THERM2_COEF_0  	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */ 
{&ten_a.ad.comp_t1[1]		, TENSION_a_THERM2_COEF_1  	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */ 
{&ten_a.ad.comp_t1[2]		, TENSION_a_THERM2_COEF_2  	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */ 
{&ten_a.ad.comp_t1[3]		, TENSION_a_THERM2_COEF_3 	},   /* Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */ 

{&ten_a.ad.comp_t1[0]		, TENSION_a_THERM1_COEF_0  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */ 
{&ten_a.ad.comp_t1[1]		, TENSION_a_THERM1_COEF_1  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */ 
{&ten_a.ad.comp_t1[2]		, TENSION_a_THERM1_COEF_2  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */ 
{&ten_a.ad.comp_t1[3]		, TENSION_a_THERM1_COEF_3  	},   /* Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */ 
};
/* **************************************************************************************
 * void* tension_param_lookup_ptr(u32 id_num);
 * @brief	: Lookup the pointer to the parameter in the struct, given the id number
 * @param	: id_num = id number of the parameter (might not be the index in the table)
 * @return	: NULL = Not found, otherwise the pointer
 * ************************************************************************************** */
void* tension_param_lookup_ptr(u32 id_num)
{
	struct PARAM_TENSIONLC *p = (struct PARAM_TENSIONLC*)&ptentbl[0];
	while (p < &ptentbl[PARAM_LIST_CT_TENSION_a])
	{
		if (p->id == id_num) return p->p;
	}
	return NULL;
}

