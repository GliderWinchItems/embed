/******************************************************************************
* File Name          : temp_calc.c
* Date First Issued  : 04/19/2014
* Board              : RxT6
* Description        : Computer deg C from filtered thermistor reading
*******************************************************************************/

#include <math.h>
#include "adcsensor_eng.h"

#define B	3380.0	 // Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
#define RS	10.0	 // Series resistor, fixed (K ohms)
#define RO	10.0	 // Thermistor room temp resistance (K ohms)
#define TREF	298.0	 // Reference temp for thermistor

/* **************************************************************************************
 * void temp_calc(void);
 * @brief	: Compute temperature from double filtered/deciimated adc readings
 * @param	; Readings
 * @return	: Degrees Centigrade * 10
 * ************************************************************************************** */
void temp_calc(void)
{
	float reading;	// Input reading
	float r;	// Thermistor resistance
	float ratio;	// R/Ro
	float Oh;
	float T;

	if (adc_temp_flag == 1) // Did adc filtering set a new reading?
	{ // Here, yes.
		reading = adc_temperature; // Thermistor filter/decimate to 2/sec
		/* Rescale adc reading to account for CIC */
		reading = reading / (1 << CICSCALE); // Reading scale: 0-4095

		/* Compute thermistor resistance from adc reading */
		r = (reading * RS) / (4095.9 - reading);

		/* Ratio of thermistor resistance to thermistor reference resistance */
		ratio = r / RO;	

		/* Compute formula */
		Oh = log(ratio) + (B / TREF);
		T = B / Oh;
		T = (T - 273.0) * 100.0; // Convert Kelvin to Centigrade and scale upwards
				
		adc_calib_temp = T;	// Convert to fixed pt (var defined in 'adcsensor_eng.c')
		adc_temp_flag = 2;	// Show adcsensor.c a value is ready to send on CAN bus.
	}
	return;
}

