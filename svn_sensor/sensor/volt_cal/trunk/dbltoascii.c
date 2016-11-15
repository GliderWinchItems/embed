/******************************************************************************
* File Name          : dbltoascii.c
* Date First Issued  : 09/30/2015
* Board              : f103
* Description        : Convert double to ascii
*******************************************************************************/

#include "idx_v_struct_print.h"
#include "fpprint.h"
#include "printf.h"
#include "libusartstm32/usartallproto.h"


/* **************************************************************************************
 * int dbltoasci(char* p, double d, int n);
 * @brief	: Convert double to ascii
 * @param	: p = pointer to output ascii string
 * @param	: d = double to be converted
 * @param	: n = number of fractional decimal places
 * @param	: + = number of chars; - = p array too short
 * ************************************************************************************** */
int dbltoasci(char* p, double x, int n)
{

	int ret;
	int i = d;	// Get whole part
	int j = i;
	*p = 0;	// Set line length zero
	if ((d < 0) && (i == 0))
	{
		sprintf(p, "   -0.",i);	// Convert whole part
	}
	else
	{
		sprintf(p, "%5d.",i);	// Convert whole part
	}
	ret = strlen(p);
	if (j < 0) j = -j;  if (d < 0) d = -d;
	double f = (d * 1000000) - (j * 1000000); // f = fractional part
	sprintf((p+ret),"%03d",(int)f);
	return;


}
