/******************************************************************************
* File Name          : tim_clk.c
* Date First Issued  : 08/06/2015
* Board              : f103
* Description        : Count seconds, minutes, hours, for yogurt.c
*******************************************************************************/
/*

*/

#include "tim_clk.h"


/* Running time count */
int32_t timclk;	// Time count (seconds)

/* Time clk countdown. */
int32_t timctr;	// Count

/* Hour minute seconds. */
struct TIMCLK timhms;



/******************************************************************************
 * void tim_clk_init(void);
 * @brief	: Initialize TIM3 that produces interrupts used for timing measurements
*******************************************************************************/
void tim_clk_init(void)
{

	
	return;
}
/******************************************************************************
 * void tim_clk_zero(void);
 * @brief	: Set time count to zero
*******************************************************************************/
void tim_clk_ctr_zero(void)
{
	timhms.sc = 0;
	timhms.mn = 0;
	timhms.hr = 0;
	return;
}
/******************************************************************************
 * struct TIMCLK* tim_clk_hms_remaining(uint32_t ctr);
 * @brief	: Convert count of secs to hours minutes seconds
 * @return	: struct with hr mn sc
*******************************************************************************/
struct TIMCLK* tim_clk_hms_remaining(uint32_t ctr)
{
	if (ctr <= 0)
	{
		timhms.sc = 0;
		timhms.mn = 0;
		timhms.hr = 0;
		return &timhms;
	}
	timhms.hr = ctr / 3600;
	timhms.mn = (ctr - (timhms.hr * 3600)) / 60;
	timhms.sc = (ctr - (timhms.hr * 3600) - (timhms.mn * 60));
	return &timhms;
}
/******************************************************************************
 * void tim_clk_counter(void);
 * @brief	: Counter tick
*******************************************************************************/
void tim_clk_counter(void)
{
	timclk += 1;
	timctr -= 1;
		
	return;
}
