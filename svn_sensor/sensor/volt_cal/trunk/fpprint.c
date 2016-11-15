/******************************************************************************
* File Name          : fpprint.c
* Date First Issued  : 08/21/2015
* Board              : f103
* Description        : Format and print floating pt
*******************************************************************************/
/*
This is used for the F103 since the launchpad compiler was not working for 
floating pt printf (which works for the 'F4).
*/
#include <stdio.h>
#include <string.h>
#include "printf.h"
#include "fpprint.h"

/* **************************************************************************************
 * void fmtprint(int i, float f, char* p);
 * @brief	: Format floating pt and print (%8.3f)
 * @param	: i = parameter list index
 * @param	: f = floating pt number
 * @param	: p = pointer to description to follow value
 * ************************************************************************************** */
void fmtprint(int i, float f, char* p)
{
	char w[64];
	double d = f;
	fpformat(w,d);
	printf("%02d\t%s %s\n\r",i,w,p);
	return;
}
/* **************************************************************************************
 * void fpformat(char* p, double d);
 * @brief	: Format floating pt to ascii
 * @param	: p = pointer to char buffer to receive ascii result
 * @param	: d = input to be converted
 * ************************************************************************************** */
void fpformat(char* p, double d)
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
	double f = (d * 1000) - (j * 1000); // f = fractional part
	sprintf((p+ret),"%03d",(int)f);
	return;
}

