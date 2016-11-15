/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : ciccogen.h
* Creater            : deh
* Date First Issued  : 08/03/2012
* Board              : Linux PC
* Description        : Generate coefficients for FIR implementation of CIC filters
*******************************************************************************/
/*

*/

#ifndef __CICCOGEN
#define __CICCOGEN

/******************************************************************************/
void ciccogen(unsigned long long *pCoeff, int nRate, int nOrder);
/* @brief	: Compute coefficients for CIC of order nOrder, e.g 3
 * @param	: pCoeff = pointer to output table of size = (nOrder * nRate), e.g. 192;
 * @param	: nRate = number of points in "boxcar" being convolved, e.g. 64.
 * @param	: nOrder = CIC order, e.g. 3
 ******************************************************************************/

#endif

