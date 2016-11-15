/******************************************************************************
* File Name          : fmtprint.h
* Date First Issued  : 05/24/2016
* Board              :
* Description        : Fixed format for floating pt
*******************************************************************************/


#ifndef __FMTPRINT
#define __FMTPRINT

/* **********************************************************/
void fpformatn(char *p, double d, int n, int m, int q);
/* @brief	: Convert double to formatted ascii, e.g. ....-3.145
 * @param	: d = input double 
 * @param	: n = scale fraction, (e.g. 1000)
 * @param	: m = number of decimal of fraction, (e.g. 3)
 * @param	: q = number of chars total (e.g. 10)
 * @param	: p = pointer to output char buffer
*********************************************************** */

void fmtprint(int i, float f, char* p);

void fpformat(char* p, double d);


#endif
