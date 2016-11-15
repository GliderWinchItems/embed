/******************************************************************************
* File Name          : id_v_struct.c
* Date First Issued  : 03/01/2015
* Board              :
* Description        : CAN msg: Translate CAN msgs for app
*******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include "id_v_struct.h"
//#include "tmpstruct.h"
#include "db/can_db.h"


// PARAM_LIST V NUMBER_TYPE
#ifndef PARAMVTYPE
#define PARAMVTYPE
struct PRMTYP{
	uint16_t id;	// Parmeter code
	uint16_t type;	// Number type code of this parameter;
};
#endif

#define TENSION_SIZE 22
#define TENSION

#ifdef CABLE_ANGLE
const struct PRMTYP prmtyp[TENSION_SIZE] = {\
{CABLE_ANGLE_POLY_COEF_3 	  TYP_FLT   },	/* Cable_Angle: Angle approximation: polynomial coefficient 3 */
{CABLE_ANGLE_POLY_COEF_2 	  TYP_FLT   },	/* Cable_Angle: Angle approximation: polynomial coefficient 2 */
{CABLE_ANGLE_POLY_COEF_1 	  TYP_FLT   },	/* Cable_Angle: Angle approximation: polynomial coefficient 1 */
{CABLE_ANGLE_POLY_COEF_0 	  TYP_FLT   },	/* Cable_Angle: Angle approximation: polynomial coefficient 0 */
{CABLE_ANGLE_ALARM_COUNT 	  TYP_U32   },	/* Cable_Angle: Number of times to send alarm msg */
{CABLE_ANGLE_PACING_COUNT	  TYP_U32   },	/* Cable_Angle: Number of poll msgs (tension) between sending cable angle */
{CABLE_ANGLE_MIN_REQ_TENSION	  TYP_FLT   },	/* Cable_Angle: Minimum tension required for valid computation (units to match) */
{CABLE_ANGLE_RAW_AD7799_2     	  TYP_U32   },	/* Cable_angle: Reading: Raw AD7799_2 */
{CABLE_ANGLE_RAW_ADC_THERM2   	  TYP_U32   },	/* Cable_angle: Reading: Raw ADC for thermistor 2 */
{CABLE_ANGLE_RAW_ADC_THERM1   	  TYP_U32   },	/* Cable_angle: Reading: Raw ADC for thermistor 1 */
{CABLE_ANGLE_LC_COEF_13  	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
{CABLE_ANGLE_LC_COEF_12  	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
{CABLE_ANGLE_LC_COEF_11  	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
{CABLE_ANGLE_LC_COEF_10  	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
{CABLE_ANGLE_LC_COEF_03  	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
{CABLE_ANGLE_LC_COEF_02  	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
{CABLE_ANGLE_LC_COEF_01  	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
{CABLE_ANGLE_LC_COEF_00  	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
{CABLE_ANGLE_THERM2_TEMP_SCALE	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Thermistor temp scale correction */
{CABLE_ANGLE_THERM2_TEMP_OFFSET	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Thermistor temp offset correction (deg C) */
{CABLE_ANGLE_THERM2_REF_TEMP	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Reference temp for thermistor */
{CABLE_ANGLE_THERM2_R_ROOMTMP	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Thermistor room temp resistance (K ohms) */
{CABLE_ANGLE_THERM2_R_SERIES	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: Series resistor, fixed (K ohms) */
{CABLE_ANGLE_THERM2_CONST_B	  TYP_FLT   },	/* Cable_angle: Thermistor2 param: constant B */
{CABLE_ANGLE_THERM1_TEMP_SCALE	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Thermistor temp scale correction */
{CABLE_ANGLE_THERM1_TEMP_OFFSET	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Thermistor temp offset correction (deg C) */
{CABLE_ANGLE_THERM1_REF_TEMP	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Reference temp for thermistor */
{CABLE_ANGLE_THERM1_R_ROOMTMP	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Thermistor room temp resistance (K ohms) */
{CABLE_ANGLE_THERM1_R_SERIES	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: Series resistor, fixed (K ohms) */
{CABLE_ANGLE_THERM1_CONST_B	  TYP_FLT   },	/* Cable_angle: Thermistor1 param: constant B */
{CABLE_ANGLE_AD7799_2_SCALE	  TYP_FLT   },	/* Cable_angle: AD7799 scale (convert to kgf */
{CABLE_ANGLE_AD7799_2_OFFSET	  TYP_S32   },	/* Cable_angle: AD7799 offset */
#endif

#ifdef ENGINE_SENSOR
const struct PRMTYP prmtyp[TENSION_SIZE] = {\
{ENGINE_THERM1_REF_TEMP  	  TYP_FLT   },	/* Engine_sensor: Thermistor param: Reference temp for thermistor */
{ENGINE_THERM1_R_ROOMTMP 	  TYP_FLT   },	/* Engine_sensor: Thermistor param: Thermistor room temp resistance (K ohms) */
{ENGINE_THERM1_R_SERIES  	  TYP_FLT   },	/* Engine_sensor: Thermistor param: Series resistor, fixed (K ohms) */
{ENGINE_THERM1_CONST_B   	  TYP_FLT   },	/* Engine_sensor: Thermistor param: constant B */
{ENGINE_SENSOR_PRESS_SCALE	  TYP_FLT   },	/* Engine_sensor: Manifold pressure  scale (inch Hg) */
{ENGINE_SENSOR_PRESS_OFFSET	  TYP_FLT   },	/* Engine_sensor: Manifold pressure offset */
{ENGINE_SENSOR_SEG_CT    	  TYP_U32   },	/* Engine_sensor: Number of black (or white) segments */
#endif

#ifdef TENSION
const struct PRMTYP prmtyp[TENSION_SIZE] = {\
{TENSION_THERM2_LC_COEF_3,	  TYP_FLT   },	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
{TENSION_THERM2_LC_COEF_2,	  TYP_FLT   },	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
{TENSION_THERM2_LC_COEF_1,	  TYP_FLT   },	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
{TENSION_THERM2_LC_COEF_0,	  TYP_FLT   },	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
{TENSION_THERM1_LC_COEF_3,	  TYP_FLT   },	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
{TENSION_THERM1_LC_COEF_2,	  TYP_FLT   },	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
{TENSION_THERM1_LC_COEF_1,	  TYP_FLT   },	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
{TENSION_THERM1_LC_COEF_0,	  TYP_FLT   },	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
{TENSION_THERM2_TEMP_SCALE,	  TYP_FLT   },	/* Tension: Thermistor2 param: Thermistor temp scale correction */
{TENSION_THERM2_TEMP_OFFSET,	  TYP_FLT   },	/* Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */
{TENSION_THERM2_REF_TEMP ,	  TYP_FLT   },	/* Tension: Thermistor2 param: Reference temp for thermistor */
{TENSION_THERM2_R_ROOMTMP,	  TYP_FLT   },	/* Tension: Thermistor2 param: Thermistor room temp resistance (K ohms) */
{TENSION_THERM2_R_SERIES ,	  TYP_FLT   },	/* Tension: Thermistor2 param: Series resistor, fixed (K ohms) */
{TENSION_THERM2_CONST_B  ,	  TYP_FLT   },	/* Tension: Thermistor2 param: constant B */
{TENSION_THERM1_TEMP_SCALE,	  TYP_FLT   },	/* Tension: Thermistor1 param: Thermistor temp scale correction */
{TENSION_THERM1_TEMP_OFFSET,	  TYP_FLT   },	/* Tension: Thermistor1 param: Thermistor temp offset correction (deg C) */
{TENSION_THERM1_REF_TEMP ,	  TYP_FLT   },	/* Tension: Thermistor1 param: Reference temp for thermistor */
{TENSION_THERM1_R_ROOMTMP,	  TYP_FLT   },	/* Tension: Thermistor1 param: Thermistor room temp resistance (K ohms) */
{TENSION_THERM1_R_SERIES ,	  TYP_FLT   },	/* Tension: Thermistor1 param: Series resistor, fixed (K ohms) */
{TENSION_THERM1_CONST_B  ,	  TYP_FLT   },	/* Tension: Thermistor1 param: constant B */
{TENSION_AD7799_1_SCALE  ,	  TYP_FLT   },	/* Tension: AD7799 #1 Scale (convert to kgf) */
{TENSION_AD7799_1_OFFSET ,	  TYP_S32   },	/* Tension: AD7799 offset */
};
#endif
/* ------------- End of generated example ------------------------------------------------------------------------------------------------------ */

/* ============== This is where the id versus struct gets setup for the app ============================================= */
struct TENSIONLC ten11;	// struct with TENSION function parameters

/* Table app program sets up to relate parameter identification number with struct */
struct PARAMIDPTR paramidptrflash[TENSION_SIZE] = {\
{TENSION_AD7799_1_OFFSET, 	&ten11.ad.offset},	/* Tension: AD7799 offset */
{TENSION_AD7799_1_SCALE,  	&ten11.ad.scale},	/* Tension: AD7799 #1 Scale (convert to kgf) */
{TENSION_THERM1_CONST_B,  	&ten11.ad.tp[0].B},	/* Tension: Thermistor1 param: constant B */
{TENSION_THERM1_R_SERIES, 	&ten11.ad.tp[0].R0},	/* Tension: Thermistor1 param: Series resistor, fixed (K ohms) */
{TENSION_THERM1_R_ROOMTMP,	&ten11.ad.tp[0].RS},	/* Tension: Thermistor1 param: Thermistor room temp resistance (K ohms) */
{TENSION_THERM1_REF_TEMP, 	&ten11.ad.tp[0].TREF},	/* Tension: Thermistor1 param: Reference temp for thermistor */
{TENSION_THERM1_TEMP_OFFSET,	&ten11.ad.tp[0].os.offset},/* Tension: Thermistor1 param: Thermistor temp offset correction (deg C) */
{TENSION_THERM1_TEMP_SCALE,	&ten11.ad.tp[0].os.scale},/* Tension: Thermistor1 param: Thermistor temp scale correction */
{TENSION_THERM2_CONST_B,  	&ten11.ad.tp[1].B},	/* Tension: Thermistor2 param: constant B */
{TENSION_THERM2_R_SERIES, 	&ten11.ad.tp[1].RS},	/* Tension: Thermistor2 param: Series resistor, fixed (K ohms) */
{TENSION_THERM2_R_ROOMTMP,	&ten11.ad.tp[1].R0},	/* Tension: Thermistor2 param: Thermistor room temp resistance (K ohms) */
{TENSION_THERM2_REF_TEMP, 	&ten11.ad.tp[1].TREF},	/* Tension: Thermistor2 param: Reference temp for thermistor */
{TENSION_THERM2_TEMP_OFFSET,	&ten11.ad.tp[1].TREF},	/* Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */
{TENSION_THERM2_TEMP_SCALE,	&ten11.ad.tp[1].os.scale},/* Tension: Thermistor2 param: Thermistor temp scale correction */
{TENSION_THERM1_LC_COEF_0,	&ten11.ad.comp_t1[0]},	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
{TENSION_THERM1_LC_COEF_1,	&ten11.ad.comp_t1[1]},	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
{TENSION_THERM1_LC_COEF_2,	&ten11.ad.comp_t1[2]},	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
{TENSION_THERM1_LC_COEF_3,	&ten11.ad.comp_t1[3]},	/* Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
{TENSION_THERM2_LC_COEF_0,	&ten11.ad.comp_t2[0]},	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
{TENSION_THERM2_LC_COEF_1,	&ten11.ad.comp_t2[1]},	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
{TENSION_THERM2_LC_COEF_2,	&ten11.ad.comp_t2[2]},	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
{TENSION_THERM2_LC_COEF_3,	&ten11.ad.comp_t2[3]},	/* Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
};

struct PARAMIDPTR paramidptrid[TENSION_SIZE];		// List of ID v PTR sorted by ID

struct TENSIONLC tensionlc1;

/*******************************************************************************
 * static int cmpfuncID (const void* a, const void* b);
 * @brief 	: Compare function for => bsearch <= (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncID (const void* a, const void* b)
{
	const uint16_t *pA = (uint16_t *)a;	// 'a' points to 'key'
	const struct PRMTYP* B = (struct PRMTYP*)b; // 'b' pts to sorted list
	struct PRMTYP* pB = *(struct PRMTYP**)B;

	/* uint16_t needed to handle +1,0,-1 comparison. */
	uint16_t aa = *pA;	// Search on this value
	uint16_t bb = pB->id;	// parameter id number

	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}

/*******************************************************************************
 * static int cmpfuncPTR (const void* a, const void* b);
 * @brief 	: Compare function for => bsearch <= (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncPTR (const void* a, const void* b)
{
	const uint16_t *pA = (uint16_t *)a;	// 'a' points to 'key'
	const struct PARAMIDPTR* B = (struct PARAMIDPTR*)b; // 'b' pts to sorted list
	struct PARAMIDPTR* pB = *(struct PARAMIDPTR**)B;

	/* uint16_t needed to handle +1,0,-1 comparison. */
	uint16_t aa = *pA;	// Search on this value
	uint16_t bb = pB->id;	// parameter id number

	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}
/******************************************************************************
 * void* id_v_struct_getlistptr(void);
 * @brief	: Return the sorted list pointer
 * @return	: Pointer to beginning of struct
 ******************************************************************************/
void* id_v_struct_getlistptr(void)
{
	return &tensionlc1;
}
/******************************************************************************
 * struct TENSIONLC * id_v_struct_getTENSIONLCptr(void);
 * @brief	: Return the sorted list pointer
 * @return	: Pointer to beginning of struct
 ******************************************************************************/
struct TENSIONLC * id_v_struct_getTENSIONLCptr(void)
{
	return (void*)paramidptrid;
}
/******************************************************************************
 * void id_v_struct_init(void);
 * @brief	: Copy flash to sram and sort lists
 ******************************************************************************/
static uint32_t init_flag = 0;	

void id_v_struct_init(void)
{
	/* Copy flash into sram */
	paramidptrid = paramidptrflash;
	
	/* Sort on id number. */
	qsort(&paramidptrid, TENSION_SIZE, sizeof(struct PARAMIDPTR*), cmpfuncPTR);
		
	init_flag = 1;
	return;
}
/******************************************************************************
 * void* id_v_struct_getptr(uint32_t* id);
 * @brief	: Find pointer to struct element, when given the id number of the element
 * @param	: id = pointer to id number of element in the struct
 * @return	: pointer to struct element; NULL = no match 
 ******************************************************************************/
void* id_v_struct_getptr(uint32_t* id)
{
	if (init_flag == 0) id_v_struct_init(); // Init if not already done

	/* Look up identification number.  Return with pointer to table entry with match */
	return (void*)bsearch(&paramidptrid, id, TENSION_SIZE, sizeof(struct PARAMIDPTR*), cmpfuncPTR);
}
/******************************************************************************
 * uint32_t id_v_struct_gettype(uint32_t* id);
 * @brief	: Get parameter type code, given the parameter id
 * @param	: id = pointer to id number of element in the struct
 * @return	: type code; negative = no match 
 ******************************************************************************/
uint32_t id_v_struct_gettype(uint32_t* id)
{
	if (init_flag == 0) id_v_struct_init(); // Init if not already done

	/* Search for ptr, return id number. */
	struct PARMIDTYPE* p = bsearch(&prmtyp, id, TENSION_SIZE, sizeof(struct PARMIDTYPE*), cmpfuncID);
	if (p == 0) return -1; 	// No match
	return p->type;	// Return type code
}

