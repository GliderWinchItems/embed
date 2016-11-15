/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : FirLp2.h
* Creater            : deh
* Date First Issued  : 07/21/2012
* Board              : PC
* Description        : Generate coefficients for Fir LP filter
*******************************************************************************/

#ifndef __FIRLP
#define __FIRLP

#define HAMMING 0
#define HANNING 1
#define BLACKMAN 2
#define RAISEDCOSINE 3

/* Coefficients */
struct FIRLPCO2
{
	double* dpHBase;	// Base address of coefficient array "H"
	int nY;			// Index into data array
	int nSze;		// Size of Fir filter 
	int nHalf;		// Rounded-down, half size
};

/* Data */
struct FIRLP
{
	double* dpBase;		// Base address of allocated data array
	double* dpHiEnd;	// High end of array (used for wraparound check)
	int nY;			// Index into data array
	int nSze;		// Size of Fir filter 
	int nHalf;		// Rounded-down, half size
};

/******************************************************************************/
double FirLp(struct FIRLPCO2 *a,struct FIRLP *d,double dData);
/* @brief	: Compute filter output
 * @param	: FIRLPCO2 *a: struct with pointers and parameters for coefficient array
 * @param	: FIRL *d: struct with pointers and parameters for data input buffer
 * @param	: dData: New input data point
 * @return	: Filter output
 ******************************************************************************/
void FirLpInit(struct FIRLPCO2 *a, int nSize, double dCutoff, int w);
/* @brief	: Setup the array of coefficients
 * @param	: nSize - number of taps
 * @param	: dCutoff - cutoff freq
 * @param	: w - Type of windowing
 ******************************************************************************/


#endif

