/******************************************************************************
* File Name          : temp_calc_param.h
* Date First Issued  : 02/14/2015
* Board              : ...
* Description        : Computer deg C from filtered thermistor reading (double)
*******************************************************************************/

#ifndef __TEMP_CALC_PARAM
#define __TEMP_CALC_PARAM

#include "common_misc.h"
#include "common_can.h"
#include "../../../../sensor/tension/trunk/tension_idx_v_struct.h"


/* **************************************************************************************/
double temp_calc_param_dbl(int adcreading, struct THERMPARAM *p);
/* @brief	: Compute temperature from adc reading ('d' = double precision)
 * @param	; adcreading,  scaled (0 - 4095) if filtered/averaged
 * @return	: Degrees Centigrade
 * ************************************************************************************** */


#endif 

