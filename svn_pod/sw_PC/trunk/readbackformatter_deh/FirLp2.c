/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : FirLp.c
* Creater            : deh
* Date First Issued  : 07/21/2012
* Board              : PC
* Description        : Generate coefficients for Fir LP filter
*******************************************************************************/

#include "FirLp2.h"
#include <malloc.h>
#include <math.h>

/******************************************************************************
 * static Common(struct FIRLPCO2 *a, int nSize, double dCutOff, double dP1, double dP2, double dP3);
 * @brief	: Allocate memory and setup the array of coefficients
 * @param	: FIRCO2 *a: pointer to struct with pointers and parameters for coefficient array.
 * @param	: nSize: number of taps
 * @param	: dCutoff: cutoff freq
 * @param	: dP1, dP2, dP3 are coefficients passed in for the type of windowing
 ******************************************************************************/
static Common(struct FIRLPCO2 *a, int nSize, double dCutOff, double dP1, double dP2, double dP3)
{
	int i;
	double pi = 3.14159265358979323846264;
	double dI,dS;
	double* dpC;
	double dOmegaC;
	double dOffSet;

	int nBit0;		// Odd/Even bit
	int nHalf1;		// Handle nSize odd case.

	a->nHalf = nSize/2;		// Rounded-down, half size
	nBit0 = (nSize & 0x01);
	nHalf1 = nBit0 + a->nHalf;		// Handle nSize odd case.

// Allocate an array for filter coefficients 
// (symmetrical, therefore only half size required)
	dpC = malloc(sizeof( double[nHalf1]));	// Low end of coefficient array
	a->dpHBase = dpC;	// Save base addr


// Compute coefficients for filter
// and scale with window function
	dOmegaC = dCutOff * pi * 2.0;
	double x = nSize;
	dOffSet = 0.5 - (x/2);

	for (i = 0; i < nHalf1; i++)
	{
		dI = (i + dOffSet);
		dS = dI * dOmegaC;
		if (dS != 0.0) 
		{
	// Basic rectangular filter
			*(a->dpHBase + i) = sin(dS)/dS;
		}
		else
			*(a->dpHBase + i) = 1.0;
	// Apply windowing
		*(a->dpHBase + i) *= dP1 - dP2 * cos( (2 * pi * i)/(nSize-1) ) +
			dP3 * cos( (4 * pi * i)/(nSize-1) );
	}
//	if ( nBit0 != 0 )
//	{	// Coefficient for odd case
//		*(dpHBase + i) = dP1 + dP2 * 1.0/(nSize-1) -
//			dP3 * 1.0/(nSize-1) ;
//	}

	// Scale so that total equals 1.00
	double dX = 0;

	for (i = 0; i < a->nHalf; i++)// Sum original array
		dX += *(a->dpHBase + i);
	dX *= 2;	// Symmetrical
	if ( nBit0 != 0 )
	{	// Handle odd case
		dX += *(a->dpHBase + i);
	}

	for (i = 0; i < nHalf1; i++)// Scale array
		*(a->dpHBase + i) *= 1.0/dX;

// Double check **TEST***
	dX = 0;
	for (i = 0; i < a->nHalf; i++)// Sum original array
		dX += *(a->dpHBase + i);
	dX *= 2;	// Symmetrical
	if ( nBit0 != 0 )
	{	// Handle odd case
		dX += *(a->dpHBase + i);
	}

// Array index initialize
	a->nY = 0;	

	return;

}
/******************************************************************************
 * void FirLpInit(struct FIRLPCO2 *a, int nSize, double dCutoff, int w);
 * @brief	: Initialize--Setup the array of coefficients
 * @param	: FIRCO2 *a: pointer to struct with pointers and parameters for coefficient array.
 * @param	: nSize - number of taps
 * @param	: dCutoff - cutoff freq
 * @param	: w - Type of windowing
 ******************************************************************************/
void FirLpInit(struct FIRLPCO2 *a, int nSize, double dCutoff, int w)
{
	switch (w)
	{
	case HAMMING:		// 0
		Common(a, nSize, dCutoff,0.54,0.46, 0.0);// Hamming window
		break;

	case RAISEDCOSINE: 	// 3
	case HANNING:		// 1
		Common(a, nSize, dCutoff,0.5,0.5, 0.0);	// Hanning window (raised cosine)
		break;

	case BLACKMAN:		// 2
		Common(a, nSize, dCutoff,0.42,0.5,0.08);	// Blackman window
		break;
	
	default:
		Common(a, nSize, dCutoff,0.54,0.46, 0.0); // Default to Hamming window since I met him
		break;
	
	}
	return;
}
/******************************************************************************
 * double FirLp(struct FIRLPCO2 *a,struct FIRLP *d,double dData);
 * @brief	: Compute filter output
 * @param	: FIRLPCO2 *a: struct with pointers and parameters for coefficient array
 * @param	: FIRL *d: struct with pointers and parameters for data input buffer
 * @param	: dData: New input data point
 * @return	: Filter output
 ******************************************************************************/
double FirLp(struct FIRLPCO2 *a,struct FIRLP *d,double dData)
{
	int i;
	int j,jj;
	double dSum = 0;
	
	j = d->nY;			// j  works towards increasing indices
	jj = d->nY - 1;			// jj works with decreasing indices
	*(d->dpBase + d->nY++) = dData;	// Circular buffer incoming data
	if (d->nY >= d->nSze) d->nY = 0;	// Check for wrap around
	if (jj < 0) jj = d->nSze-1;	// Check for wrap around

	/* 
	At this point d->nY points to next-available-storage location
	j  points to most recently stored data point
	jj points to oldest stored data point.
	*/
	
	/* 
	Multiply and accumulate each data in the incoming circular buffer 
	with each coefficient.  Since coefficients are symmetrical, reuse them
	by working from top and bottom ends of circular buffered data.  In so
	doing, the precision issue is helped as the smallest coefficients are added
	first, thus reducing the loss of precision when adding small floating numbers
	to big ones.
	*/
	
	for ( i = 0; i < d->nHalf; i++)
	{
		dSum = dSum + (*(a->dpHBase + i) * *(d->dpBase + j++)  );
		if (j >= d->nSze) j = 0;		
		dSum = dSum + (*(a->dpHBase + i) * *(d->dpBase + jj--) );
		if (jj < 0) jj = d->nSze-1;
	}
	if ( (d->nSze & 0x01) != 0 ) // Check odd size bit
	{	// Here, odd number of taps
		dSum = dSum + (*(a->dpHBase + i) * *(d->dpBase + j++)  );
		if (j >= d->nSze) j = 0;		
	}

	return dSum;

}
