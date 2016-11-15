/******************************************************************************
* File Name          : queue_dbl.h
* Date First Issued  : 06/05/2016
* Board              : 
* Description        : queue/circular buffer  for passing doubles
*******************************************************************************/
#ifndef __QUEUE_DBL
#define __QUEUE_DBL

/* For passing two word values between interrupt levels */
struct TENQUEUEDBL
{
	double* pin;	// Pointer to incoming values
	double* pout;	// Pointer to outgoing values
	double* pbegin;	// Pointer to beginning of buffer
	double* pend;	// Pointer to end+1 of buffer
};

/* **************************************************************************************/
struct TENQUEUEDBL* queue_dbl_init(int size); 
/* @brief	: Add to circular buffer
 * @param	: size = number of doubles in buffer
 * ************************************************************************************** */
void queue_dbl_add(struct TENQUEUEDBL* p, double dval); 
/* @brief	: Add to circular buffer
 * @param	: p = pointer to circular buffer
 * @param	: dval = value (double) to be added to buffer
 * ************************************************************************************** */
double* queue_dbl_get(struct TENQUEUEDBL* p); 
/* @brief	: Get value from circular buffer
 * @param	: p = pointer to circular buffer
 * @return	: NULL = no new values, Not null = pointer to value
 * ************************************************************************************** */

#endif 
