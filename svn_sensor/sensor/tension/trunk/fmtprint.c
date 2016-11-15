/******************************************************************************
* File Name          : fmtprint.c
* Date First Issued  : 05/24/2016
* Board              :
* Description        : Fixed format for floating pt
*******************************************************************************/

#include <stdio.h>
#include "fmtprint.h"

void fpformat(char* p, double d)
{
	fpformatn(p, d, 1000, 3, 12);
}
/* **********************************************************
 * void fpformatn(char *p, double d, int n, int m, int q);
 * @brief	: Convert double to formatted ascii, (e.g. ....-3.145)
 * @param	: d = input double 
 * @param	: n = scale fraction, (e.g. 1000)
 * @param	: m = number of decimals of fraction, (e.g. 3)
 * @param	: q = number of chars total (e.g. 10)
 * @param	: p = pointer to output char buffer
*********************************************************** */
void fpformatn(char *p, double d, int n, int m, int q)
{
	#define SZ 32
	char x[SZ];
	int i;
	char sign = ' ';
	double dt = n * d + 0.5;	// Round *up*

	/* Check if long long is needed. */
	if ((dt > 2147483647.0) || (dt < 2147483648.0))
	{ // Here, long long required 
		long long ll = dt;
		long long llt;
		if (ll < 0) {ll = -ll; sign = '-';}

		x[SZ-1] = 0;	// String terminator
		for (i = (SZ-2) ; i >= (SZ-1-m); i--)
		{
			llt = ll/10;
			x[i] = ll - llt*10 + '0';
			ll = llt;
		}
		x[i--] = '.';
		if (ll == 0)
		{
			x[i--] = '0';
		}
		else
		{
	
			while ((ll != 0) && (i > 0))
			{
				llt = ll/10;
				x[i--] = ll - llt*10 + '0';
				ll = llt;
			}
		}
	}
	else
	{ // Here long will suffice
		long l = dt;
		long lt;
		if (l < 0) {l = -l; sign = '-';}

		x[SZ-1] = 0;	// String terminator
		for (i = (SZ-2) ; i >= (SZ-1-m); i--)
		{
			lt = l/10;
			x[i] = l - lt*10 + '0';
			l = lt;
		}
		x[i--] = '.';
		if (l == 0)
		{
			x[i--] = '0';
		}
		else
		{
	
			while ((l != 0) && (i > 0))
			{
				lt = l/10;
				x[i--] = l - lt*10 + '0';
				l = lt;
			}
		}
	}
	x[i--] = sign;

	while (i >= 0)	// Pad leading blanks
	{
		x[i--] = ' ';
	}
	// Copy to output in reverse order of array. */
	for (i = SZ - q; i < SZ; i++)
	{
		*p++ = x[i];
	}
	return;
}

void fmtprint(int i, float f, char* p)
{
	char w[64];
	double d = f;
	fpformat(w,d);
	printf("%02d\t%s %s\n\r",i,w,p);
	return;
}

