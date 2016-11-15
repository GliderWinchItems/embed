/******************************************************************************
* File Name          : iir_1.c
* Date First Issued  : 09/29/2015
* Board              : --
* Description        : Single pole iir filter
*******************************************************************************/


/******************************************************************************
 * double iir_1(double adc, struct IIR_1* p);
 * @brief 	: Single pole IIR Filter 
 * @param	: ADC reading 
 * @param	: p = pointer to struct IIR_1
 * return	: filtered value
*******************************************************************************/
double iir_1(double adc, struct IIR_1* p)
{
	/* Initialize if not previously done. */
	if (p->sw == 0)
	{ 
		p->sw    = 1; 
		p->exp   = exp( -(1.0/(p->a) ));
		p->scale = (1.0 - p->exp);
		p->z     = adc;
	}
	/* filter */
	tmp = adc + p->z * p->exp;
	p->z = tmp;
	tmp = (tmp * p->scale);
	return tmp;
}

