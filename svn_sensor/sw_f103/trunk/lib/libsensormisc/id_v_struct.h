/******************************************************************************
* File Name          : id_v_struct.h
* Date First Issued  : 03/01/2015
* Board              :
* Description        : CAN msg: Translate CAN msgs for app
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"
//#include "tmpstruct.h"

#ifndef __ID_V_STRUCT
#define __ID_V_STRUCT


  // -----------------------------------------------------------------------------------------------

#ifndef STRUCT_SCLOFF
#define STRUCT_SCLOFF
struct SCLOFF
{
	float	offset;	
	float	scale;
};
#endif


 // Thermistor parameters for converting ADC readings to temperature
 #ifndef STRUCT_THERMPARAM
 #define STRUCT_THERMPARAM
 struct THERMPARAM
 {   //                   	   default values    description
	float B;		//	3380.0	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;		//	10.0	// Series resistor, fixed (K ohms)
	float R0;		//	10.0	// Thermistor room temp resistance (K ohms)
	float TREF;		//	298.0	// Reference temp for thermistor
	struct SCLOFF	os;	//      0.0,1.0	// Therm temp correction offset	1.0 Therm correction scale
 };
 #endif




 #ifndef STRUCT_AD7799
 #define STRUCT_AD7799
 struct AD7799PARAM
 {
	int 	 offset;	// AD7799 offset
	float 	 scale;		// AD7799 scale	
	struct THERMPARAM tp[2];// Two thermistor parameter sets
	float	comp_t1[4];	// Temp compensation for thermistor 1
	float	comp_t2[4];	// Temp compensation for thermistor 2
 };
 #endif

 // Tension 
 #ifndef STRUCT_TENSIONLC
 #define STRUCT_TENSIONLC

 struct TENSIONLC
 {
 	unsigned int crc;		// crc-32 placed by loader
	unsigned int version;		// struct version number
	struct AD7799PARAM ad;		// Parameters for one AD7799 (w thermistors)
	char c[4*8];			// ascii mini-description
 };
 #endif


 #ifndef STRUCT_PARAMIDPTR
 #define STRUCT_PARAMIDPTR
struct PARAMIDPTR {
	uint16_t id;
	void*	ptr;
};
#endif




/******************************************************************************/
void id_v_struct_init(void);
/* @brief	: Copy flash to sram and sort lists
 ******************************************************************************/
void* id_v_struct_getptr(uint32_t* id);
/* @brief	: Find pointer to struct element, when given the id number of the element
 * @param	: id = pointer to id number of element in the struct
 * @return	: pointer to struct element; NULL = no match 
 ******************************************************************************/
uint32_t id_v_struct_gettype(uint32_t* id);
/* @brief	: Get parameter type code, given the parameter id
 * @param	: id = pointer to id number of element in the struct
 * @return	: type code; negative = no match 
 ******************************************************************************/


#endif


