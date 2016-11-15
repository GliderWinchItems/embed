/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : calibration_init.c
* Hackerees          : deh
* Date First Issued  : 12/14/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Initialize calibration block
*******************************************************************************/
#include "calibration.h"

/******************************************************************************
 * void calibration_init_default(struct CALBLOCK *p);
 * @brief	: Initialize calibration block
 * @param	: Pointer to the calibrationblock
 ******************************************************************************/
void calibration_init_default(struct CALBLOCK *p)
{
	p->version = CALVERSION;		/* Calibration version number 		*/
	p->adcbot = 76590;			/* Bottom cell direct measurement 	*/
	p->adctop = 142253;			/* Top cell direct measurement 		*/
	p->celllow = 3450;			/* Cell low voltage limit (mv)		*/
	p->adcppm = -76;			/* ADC offset for temp compensation table lookup of 32 KHz osc */
	p->tmpoff = -380;			/* Temp offset conversion table lookup (see also '../devices/adcthermtmp.c') */
	p->ppmoff = 310;			/* ppm * 100 offset for 32 KHz osc nominal freq */
	p->accel_offset[0] = 2351;		/* Z axis zero offset	*/
	p->accel_offset[1] = 2308;		/* Y axis zero offset	*/
	p->accel_offset[2] = 2307;		/* X axis zero offset	*/
	p->accel_scale[0] = 351,		/* Z axis scale for 1g * 100 */
	p->accel_scale[1] = 355,		/* Y axis scale for 1g * 100 */
	p->accel_scale[2] = 347;		/* X axis scale for 1g * 100 */
	p->load_cell = 35268;			/* AD7799: counts per kg*10 */
	p->load_cell_zero = 515;		/* AD7799: zero adjust */
	p->xtal_To = 2447;			/* 32 KHz xtal turnpoint temp (25.0 C nominal, scaled to 2500) */
	p->xtal_alpha = 4274;			/* 32 KHz xtal alpha coefficient (0.034 nominal, i.e. scaled to 3400) */
	p->xtal_a = 15647;			/* Temp compensation polynomial coefficient (scaled) */
	p->xtal_b = -5561496;			/* Temp compensation polynomial coefficient (scaled) */
	p->xtal_c = 171934971;			/* Temp compensation polynomial coefficient (scaled) */
	p->xtal_d = 73777;			/* Temp compensation polynomial coefficient (scaled) */
	p->xtal_t = 645;			/* Temperature (C*100) offset that is input to polynomial computation */
	p->xtal_o = 100;			/* Fixed offset (ppb) that is applied after polynomial computation 32 KHz */
	p->xtal_o = 0;				/* Fixed offset (ppb) that is applied after polynomial computation  8 KHz */

	return ;
};

