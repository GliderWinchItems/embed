/******************************************************************************
* File Name          : can_cnvt.c
* Date First Issued  : 09/09/2014
* Board              : Discovery F4
* Description        : Convert payload bytes to/from shorts and longs
*******************************************************************************/
#include "can_cnvt.h"

/******************************************************************************
 * u16 can_cnvt_u16(struct CANRCVBUF* pcan, unsigned int n);
 * s16 can_cnvt_s16(struct CANRCVBUF* pcan, unsigned int n)
 * @brief 	: Convert bytes from payload to short
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @return	: short
*******************************************************************************/
u16 can_cnvt_u16(struct CANRCVBUF* pcan, unsigned int n)
{
	return ( (pcan->cd.uc[(n+1)] << 8) | pcan->cd.uc[(n+0)] );
}
s16 can_cnvt_s16(struct CANRCVBUF* pcan, unsigned int n)
{
	return ( (pcan->cd.sc[(n+1)] << 8) | pcan->cd.uc[(n+0)] );
}
/******************************************************************************
 * u32 can_cnvt_u32(struct CANRCVBUF* pcan, unsigned int n);
 * s32 can_cnvt_s32(struct CANRCVBUF* pcan, unsigned int n)
 * @brief 	: Convert bytes from payload to int
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @return	: int
*******************************************************************************/
u32 can_cnvt_u32(struct CANRCVBUF* pcan, unsigned int n)
{
	return ( (pcan->cd.uc[(n+3)] << 24) | pcan->cd.uc[(n+2)] << 16 | (pcan->cd.uc[(n+1)] << 8) | pcan->cd.uc[(n+0)] );
}
s32 can_cnvt_s32(struct CANRCVBUF* pcan, unsigned int n)
{
	return ( (pcan->cd.sc[(n+3)] << 24) | pcan->cd.uc[(n+2)] << 16 | (pcan->cd.uc[(n+1)] << 8) | pcan->cd.uc[(n+0)] );
}
/******************************************************************************
 * void can_cnvt_put_16(struct CANRCVBUF* pcan, unsigned int n, u16 w);
 * @brief 	: Store word in payload (unsigned, signed)
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @param	: w = short or unsigned short to be stored
*******************************************************************************/
void can_cnvt_put_16(struct CANRCVBUF* pcan, unsigned int n, u16 w)
{
	if (n > 5) return;
	pcan->cd.uc[n+0] =  w;
	pcan->cd.uc[n+1] = (w >> 8);
	return;
}

/******************************************************************************
 * void can_cnvt_put_32(struct CANRCVBUF* pcan, unsigned int n, u32 w);
 * @brief 	: Store word in payload (unsigned, signed)
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @param	: w = int or unsigned int to be stored
*******************************************************************************/
void can_cnvt_put_32(struct CANRCVBUF* pcan, unsigned int n, u32 w)
{
	if (n > 3) return;
	pcan->cd.uc[n+0] = (w >>  0);
	pcan->cd.uc[n+1] = (w >>  8);
	pcan->cd.uc[n+2] = (w >> 16);
	pcan->cd.uc[n+3] = (w >> 24);
	return;
}


