/******************************************************************************
* File Name          : tension_readings_ptr.c
* Date First Issued  : 06/07/2016
* Board              :
* Description        : Pointer to reading versus code
*******************************************************************************/
/* 
The pointers point to the first instance of 'tension_a_functionS.h'
The use of the table for additional instances subtract the base and use the offset.
*/

#include <stdint.h>
#include "../../../../svn_common/trunk/db/gen_db.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "tension_readings_ptr.h"
#include "tension_a_functionS.h"

const struct READINGSPTR tension_readings_ptr[] = {\
{&ten_f[0].thrm[0],TENSION_READ_FILTADC_THERM1},	/* Filtered reading converted to double */
{&ten_f[0].thrm[1],TENSION_READ_FILTADC_THERM2},	/* Filtered reading converted to double */
\
{&ten_f[0].degX[0],TENSION_READ_FORMCAL_THERM1},	/* Tension: READING: Formula Calibrated temperaure for Thermistor 1 */
{&ten_f[0].degX[1],TENSION_READ_FORMCAL_THERM2},	/* Tension: READING: Formula Calibrated temperaure for Thermistor 2 */
\
{&ten_f[0].degC[0],TENSION_READ_POLYCAL_THERM1},	/* Tension: READING: Polynomial adjusted temperaure for Thermistor 1 */
{&ten_f[0].degC[1],TENSION_READ_POLYCAL_THERM2},	/* Tension: READING: Polynomial temperaure for Thermistor 2 */
\
{&ten_f[0].lgr,TENSION_READ_AD7799_LGR},		/* Tension: READING: int32_t lgr; last_good_reading (no filtering or adjustments) */
{&ten_f[0].ten_iircal[0],TENSION_READ_AD7799_CALIB_1},	/* Tension: READING: ten_iircal[0];  AD7799 filtered (fast) and calibrated */
{&ten_f[0].ten_iircal[1],TENSION_READ_AD7799_CALIB_2},	/* Tension: READING: ten_iircal[1];  AD7799 filtered (fast) and calibrated */
\
{&ten_f[0].cicraw, TENSION_READ_CIC_RAW},		/* Tension: READING: cic before averaging */
{&ten_f[0].cicave, TENSION_READ_CIC_AVE},		/* Tension: READING: Averaged cic readings */
{&ten_f[0].ave.n,TENSION_READ_CIC_AVE_CT},		/* Tension: READING: int32_t ave.n;  current count for above average */
\
};
