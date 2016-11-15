/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : calibration.c
* Hackerees          : deh
* Date First Issued  : 06/24/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : calibration of various pod devices
*******************************************************************************/
#include "calibration.h"
/*
08-11-2011	Added temp and ppm 
11-01-2011	Added turnpoint temp and alpha for 32 KHz xtal
11-07-2011	Added third order polynomial coefficients

*/
/* Default calibration */

/*
Temp difference between gpstest2 with ADC offsets = 0 and thermometer = + 2.0 deg F
Freq diff between gps and 32 KHz == 4.03 ppm less temp adjust of 2.07 ppm -> 1.96 ppm
*/

struct CALBLOCK strDefaultCalib =
{
CALVERSION,			/* Version number (0 = original)	*/
76590,				/* Bottom cell direct measurement 	*/
142253,				/* Top cell direct measurement 		*/
3450,				/* Cell low voltage limit (mv)		*/
-76,				/* ADC offset for temp compensation table lookup of 32 KHz osc */
-380,				/* Temp offset conversion table lookup (see also '../devices/adcthermtmp.c') */
310,				/* ppm * 100 offset for 32 KHz osc nominal freq */
{2345,				/* Z axis zero offset	*/
2302,				/* Y axis zero offset	*/
2336},				/* X axis zero offset	*/
{352,				/* Z axis scale for 1g * 100 */
356,				/* Y axis scale for 1g * 100 */
347},				/* X axis scale for 1g * 100 */
35268,				/* AD7799: counts per kg*10 */
-1186,				/* AD7799: zero adjust */
2447,				/* 32 KHz xtal turnpoint temp (25.0 C nominal, scaled to 2500) */
4274,				/* 32 KHz xtal alpha coefficient (0.034 nominal, i.e. scaled to 3400) */
15647,				/* Temp compensation polynomial coefficient (scaled) */
-5561496,			/* Temp compensation polynomial coefficient (scaled) */
171934971,			/* Temp compensation polynomial coefficient (scaled) */
73777,				/* Temp compensation polynomial coefficient (scaled) */
645,				/* Temperature (C*100) offset that is input to polynomial computation */
100,				/* Fixed offset (ppb) that is applied after polynomial computation 32 KHz */
0,				/* Fixed offset (ppb) that is applied after polynomial computation 8 KHz */
{},				/* Spare entries */
0				/* Checksum goes here */
};

