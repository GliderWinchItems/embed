/******************************************************************************
* File Name          : sdadc_filter.c
* Date First Issued  : 03/09/2016
* Board              : F373 
* Description        : SDADC filtering of data from DMA buffer
*******************************************************************************/
/* 

*/
#define NULL 0
#include "cic_filter_l_N2_M3_f3.h"
#include "sdadc_discovery.h"
#include "sdadc_filter.h"

struct SDADC_FILTERED_BUFF cic1[NUMBERSDADC1];
struct SDADC_FILTERED_BUFF cic2[NUMBERSDADC2];
struct SDADC_FILTERED_BUFF cic3[NUMBERSDADC3];
struct SDADC_FILTERED_BUFF test[2];

/* Option of making each individual port have a different decimate number. */
const uint8_t decimatetbl[NUMBERSDADCS][MAXPORTSPERSDADC] = { \
{4,4,4,4,4,4,4,4,4,},
{4,4,4,4,4,4,4,4,4,},
{4,4,4,4,4,4,4,4,4,},
};

// Decimation table for 2nd cic computation
const uint8_t decimatetbl2[NUMBERSDADCS][MAXPORTSPERSDADC] = { \
{64,64,64,64,64,64,64,64,64,},
{64,64,64,64,64,64,64,64,64,},
{64,64,64,64,64,64,64,64,64,},
};

struct SDADC_FILTER_INIT
{
	struct SDADC_FILTERED_BUFF *pcic;
  const struct SDADCFIX * pfix;	// SDADCx parameters 'sdadc_discovery.h'
	unsigned short discard;
};

const struct SDADC_FILTER_INIT filter_init[NUMBERSDADCS] = { \
  {
	&cic1[0],
	&sdadcfix[0],
	1
  },{
	&cic2[0],
	&sdadcfix[1],
	1
  },{
	&cic3[0],
	&sdadcfix[2],
	1
  }
};
	
void sdadc_filter_int(const struct SDADCFIX *p);
static void filter_zero(const struct SDADC_FILTER_INIT *pinit);

/******************************************************************************
 * void sdadc1_filter_init(void);
 * @brief 	: Initialize for filtering SDADC 1
*******************************************************************************/
void sdadc1_filter_init(void)
{
	sdadc1_ll_ptr = &sdadc_filter_int;
	filter_zero(&filter_init[0]);
	return;
}
/******************************************************************************
 * void sdadc2_filter_init(void);
 * @brief 	: Initialize for filtering SDADC 2
*******************************************************************************/
void sdadc2_filter_init(void)
{
	sdadc2_ll_ptr = &sdadc_filter_int;
	filter_zero(&filter_init[1]);
	return;
}
/******************************************************************************
 * void sdadc3_filter_init(void);
 * @brief 	: Initialize for filtering SDADC 3
*******************************************************************************/
void sdadc3_filter_init(void)
{
	sdadc3_ll_ptr = &sdadc_filter_int;
	filter_zero(&filter_init[2]);
	return;
}
/******************************************************************************
 * void sdadc123_filter_init(void);
 * @brief 	: Initialize for filtering SDADC 1,2,3
*******************************************************************************/
void sdadc123_filter_init(void)
{
	sdadc1_filter_init();
	sdadc2_filter_init();
	sdadc3_filter_init();
	return;
}

/******************************************************************************
 * long long* sdadc_filter_get(uint16_t sdadcnum, uint16_t portseqnum);
 * @brief 	: Check for filtered data in SDADCx porty
 * @param	: sdadcnum = SDADC number (1,2,3)
 * @param	: partnum = port *sequence* number ( 0 - (NUMBERSDADCSx-1))
*******************************************************************************/
long long* sdadc_filter_get(uint16_t sdadcnum, uint16_t portseqnum)
{
	long long *ptmp;
	struct SDADC_FILTERED_BUFF *pfb = &filter_init[sdadcnum-1].pcic[portseqnum];
	ptmp = pfb->pout;
	if (pfb->pin == ptmp) return NULL;
	pfb->pout += 1; if (pfb->pout >= &pfb->lout[LOUTSIZE]) pfb->pout = &pfb->lout[0];
	return ptmp;	
}
/******************************************************************************
 * static void filter_zero(const struct SDADC_FILTER_INIT *pinit);
 * @brief 	: Clear values
*******************************************************************************/
static void filter_zero(const struct SDADC_FILTER_INIT *pinit)
{
	int i,j;

	/* Initialize the structs that hold the CIC filtering intermediate values. */
	for (i = 0; i < pinit->pfix->nadc; i++)	// For each SDADC port
	{
		pinit->pcic[i].pin  = &pinit->pcic[i].lout[0]; // Circular buffer pointer
		pinit->pcic[i].pout = &pinit->pcic[i].lout[0]; // Circular buffer pointer

		pinit->pcic[i].cic.usDecimateNum = decimatetbl[pinit->pfix->sdnum][i];// Decimation number
		pinit->pcic[i].cic.usDiscard = pinit->discard;	// Initial discard count
		pinit->pcic[i].cic.usDecimateCt = 0;		// Decimation counter
		pinit->pcic[i].cic.usFlag = 0;			// 1/2 buffer flag

		pinit->pcic[i].cicll.usDecimateNum = decimatetbl2[pinit->pfix->sdnum][i];// Decimation number
		pinit->pcic[i].cicll.usDiscard = pinit->discard;// Initial discard count
		pinit->pcic[i].cicll.usDecimateCt = 0;		// Decimation counter
		pinit->pcic[i].cicll.usFlag = 0;		// 1/2 buffer flag

		for (j = 0; j < 3; j++)
		{ // Very important that the integrators begin with zero.
			pinit->pcic[i].cic.lIntegral[j] = 0;
			pinit->pcic[i].cic.lDiff[j][0] = 0;
			pinit->pcic[i].cic.lDiff[j][1] = 0;

			pinit->pcic[i].cicll.llIntegral[j] = 0;
			pinit->pcic[i].cicll.llDiff[j][0] = 0;
			pinit->pcic[i].cicll.llDiff[j][1] = 0;
		}	
	} 
	return;	
}
/* ################################################################################
 * Post DMA Low level interrupt
 * void sdadc_filter_int(const struct SDADCFIX *p);
 * @param	: p = points to struct of constants for SDADC to be handled
  ################################################################################# */
long long SDADCbug;

static struct SDADC_FILTERED_BUFF *ptbl[NUMBERSDADCS] = {&cic1[0],&cic2[0],&cic3[0]};
void sdadc_filter_int(const struct SDADCFIX *p)
{
	/* Skip if data flag is not on.  This would happen if the calibration interrupt
           coincided with the DMA interrupt (which might not be possible). */
	if (p->var->flag == 0) return;
	p->var->flag = 0;

	int i,j;
	int ret;
	int16_t *pdat = p->base;
	struct SDADC_FILTERED_BUFF *pbuff = ptbl[p->sdnum];
	struct SDADC_FILTERED_BUFF *pbase = pbuff;

	for (j = 0; j < p->nseq; j++)
	{
		pbase = pbuff;
		for (i = 0; i < p->nadc; i++)
		{
			pbase->cic.nIn = *pdat++ + 32768; // Remove bias
//TEST			pbase->cic.nIn = 65536;
			ret = cic_filter_l_N2_M3_f3(&pbase->cic);
			if (ret != 0)
			{
				pbase->cicll.nIn = pbase->cic.lout/128;// Scale stay within bounds
//SDADCbug = pbase->cicll.nIn;
				ret = cic_filter_ll_N2_M3_f3(&pbase->cicll);
				if (ret != 0)
				{
					*pbase->pin = pbase->cicll.llout; // Save filtered result in output buffer
SDADCbug = *pbase->pin;
					pbase->pin++;	// Advance pointer, check for wrap-around
					 if (pbase->pin >= &pbase->lout[LOUTSIZE]) pbase->pin = &pbase->lout[0];
				}
			}
			pbase++;	// Step to next sdadc port
		}
	}
	return;
}
