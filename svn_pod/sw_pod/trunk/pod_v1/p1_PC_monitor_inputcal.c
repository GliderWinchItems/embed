/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : p1_PC_monitor_linputcal.c
* Author             : deh
* Date First Issued  : 02/03/2012
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Input new calibration item
*******************************************************************************/
/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/lib/libusupportstm32/adc_packetize.c
@2 = svn_pod/sw_pod/trunk/pod_v1/p1_PC_monitor_batt.c
*/




#include "p1_gps_time_convert.h"
#include "p1_common.h"
#include "calendar_arith.h"
#include <time.h>




/* Subroutine prototypes */
/******************************************************************************
 * Table of names versus positions in calibration table
 ******************************************************************************/
const char *calib_name[64] = {
	"adcbot",		/* Top cell calibration * adc reading -> actual voltage (mv) */
	"adctop",		/* Bot cell calibration * adc reading -> actual voltage (mv) */
	"celllow",	/* Below this cell voltage shutdown (mv) */
	"adcppm",		/* ADC offset for temp compensation table lookup of 32 KHz osc (see also '../devices/adcppm.c') */
	"tmpoff",		/* Temp offset conversion table lookup (see also '../devices/adcthermtmp.c') */
	"ppmoff",		/* ppm * 100 offset for 32 KHz osc nominal freq */
	"accel_offset[0]",/* ADC value for zero reading */
	"accel_offset[1]",/* ADC value for zero reading */
	"accel_offset[2]",/* ADC value for zero reading */
	"accel_scale[0]",	/* Scaling for g * 100 */
	"accel_scale[1]",	/* Scaling for g * 100 */
	"accel_scale[2]",	/* Scaling for g * 100 */
	"load_cell",	/* Convert readings: counts per kg*10 */
	"load_cell_zero",	/* Trim up the zero reading */
	"xtal_To",	/* Turnpoint temperature for 32 KHz xtal (deg C * 100) */
	"xtal_alpha",	/* 32 KHz xtal temp coefficient (ppm = alpa * (T -To)^2) */
	"xtal_a",		/* Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3) (see tickadjust.c) */
	"xtal_b",		/* Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3) */
	"xtal_c",		/* Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3) */
	"xtal_d",		/* Scaled polynomial coefficient for temp compensation (a + bx +cx^2 +dx^3) */
	"xtal_t",		/* Adjustment to temperature that is input to polynomial  */
	"xtal_o",		/* Adjustment of offset after polynomial computation 32 KHz (see 'void RTC_tickadjust_poly_compute(void)' in @1) */
	"xtal_o8",	/* Adjustment of offset after polynomial computation  8 KHz (see 'void RTC_tickadjust_poly_compute(void)' in @1) */
      "spare[0]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[2]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[3]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[4]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[5]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[6]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[7]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[8]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[9]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[10]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[11]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[12]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[13]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[14]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[15]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[16]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[17]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[18]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[19]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[20]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[21]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[22]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[23]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[24]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[25]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[26]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[27]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[28]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[29]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[30]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[31]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[32]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[33]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[34]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[35]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[36]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[37]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
      "spare[38]",	/* Allow for additional future calibration entries *** BE SURE sizeof (struct CALBLOCK) = 256 *** */
	"crc",		/* Last item is crc */
};


/******************************************************************************
 * int p1_PC_monitor_inputcal(short codenumber);
 * @brief	: Input a new calibration table value
 * @param	: codenumber: command chard
 * @return	: 0 = OK, not zero = NG
 ******************************************************************************/
static int state;
static struct TWO two_f_old;
int p1_PC_monitor_inputcal(short codenumber)
{
	struct TWO two_x;	// Temp for flipping old and new

	if (codenumber == 0 ) // Reset the sequence the command has be cancelled
	{
		state = 0; return 0;
	}
	if (codenumber == 2 ) // Reset the sequence the command has be cancelled
	{ // Here, command was given to flip the saved and current calibration
		two_x = two_f_old;
		two_f_old = two_f;
		two_f = two_x;
		*( ((int *)(&strDefaultCalib)) + two_f.n1) = two_f.n2;	
		return 0;				
	}

	switch (state)
	{
	case 0:
		printf (" This has changed table position %d which is *%s*\n\r  from %d\n\r    to %d\n\r", two_f.n1, calib_name[two_f.n1],*( ((int *)(&strDefaultCalib)) + two_f.n1),two_f.n2);
		USART1_txint_send();
		state = 1;
	case 1:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() != 0) return 1;// Return: all buffers full (@1)
		USART1_txint_puts(" command p will set flag to update SD card upon sleep (command s)\n\r"); USART1_txint_send();
		state = 2;
	case 2:
		if (USART1_txint_busy() != 0) return 1;// Return: all buffers full (@1)
		USART1_txint_puts(" command w will clear the update flag so that the SD card does not update\n\r"); USART1_txint_send();
		state = 3;
	case 3:
		if (USART1_txint_busy() != 0) return 1;// Return: all buffers full (@1)
		USART1_txint_puts(" command v will flip the latest versus old values\n\r"); USART1_txint_send();
		state = 4;
		break;
	case 4:
		/* Save the old calibration */
		two_f_old.n1 = two_f.n1;	// Save position

		/* Move the new one into position in memory */
		*( ((int *)(&strDefaultCalib)) + two_f.n1) = two_f.n2;
		state = 5;
		break;
	case 5: // Keeping coming here until command has been cancelled

		break;

	default:
		break;
	
	}

	return 0;
}

