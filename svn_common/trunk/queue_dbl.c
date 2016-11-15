/******************************************************************************
* File Name          : queue_dbl.c
* Date First Issued  : 06/05/2016
* Board              : 
* Description        : queue/circular buffer  for passing doubles
*******************************************************************************/
#include <malloc.h>
#include "queue_dbl.h"

/* **************************************************************************************
 * struct TENQUEUEDBL* queue_dbl_init(int size); 
 * @brief	: Add to circular buffer
 * @param	: size = number of doubles in buffer
 * ************************************************************************************** */
struct TENQUEUEDBL* queue_dbl_init(int size)
{
	/* Get a buffer */
	struct TENQUEUEDBL* p = (struct TENQUEUEDBL*)malloc(sizeof(double) * size);
	if (p == NULL) return NULL;

	/* Set pointers */
	p->pin  = (double*)p;	// Pointer to incoming doubles
	p->pout = p->pin;	// Pointer to outgoing doubles
	p->pbegin = p->pout;	// Pointer to beginning of buffer
	p->pend = (p->pbegin + size); // Pointer to end+1
	return p;
}
/* **************************************************************************************
 * void queue_dbl_add(struct TENQUEUEDBL* p, double dval); 
 * @brief	: Add to circular buffer
 * @param	: p = pointer to circular buffer
 * @param	: dval = value (double) to be added to buffer
 * ************************************************************************************** */
void queue_dbl_add(struct TENQUEUEDBL* p, double dval)
{
	*p->pin = dval;
	p->pin += 1;
	if (p->pin >= p->pend) p->pin = p->pbegin;
	return;
};
/* **************************************************************************************
 * double* queue_dbl_get(struct TENQUEUEDBL* p); 
 * @brief	: Get value from circular buffer
 * @param	: p = pointer to circular buffer
 * @return	: NULL = no new values, Not null = pointer to value
 * ************************************************************************************** */
double* queue_dbl_get(struct TENQUEUEDBL* p)
{
	double* ptmp;
	if (p->pout == p->pin) return NULL;
	ptmp = p->pout;
	p->pout += 1;
	if (p->pout >= p->pend) p->pout = p->pbegin;
	return ptmp;
};
