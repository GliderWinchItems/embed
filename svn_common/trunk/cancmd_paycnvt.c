/******************************************************************************
* File Name          : cancmd_paycnvt.c
* Date First Issued  : 03/01/2015
* Board              :
* Description        : 
*******************************************************************************/

#include "pay_flt_cnv.h
"
//v /* Types of payload field conversions */
//v /* (dlc used to determine multiple values) */
//v /* TYPE_NAME TYPE_CODE TYPE_SIZE	*/
//v #define PY_S8	1	1-6	// (one - six)
//v #define PY_U8	2	1-6	// (one - six)
//v #define PY_S16	3	2,4,6	// (one - three)
//v #define PY_U16	4	2,4,6	// (one - three)
//v #define PY_S32	5	4
//v #define PY_U32	6	4
//v #define PY_S64_L	7	4	// Low order 4 bytes
//v #define PY_S64_H	8	4	// High order 4 bytes
//v #define PY_U64_L	9	4	// Low order 4 bytes
//v #define PY_U64_H	10	4	// High order 4 bytes
//v #define PY_FLT	11	4	// full float
//v #define PY_12flt	12	2,4,6	// 1/2 float (one - three)
//v #define PY_34flt	13	3,6	// 3/4 float (one or two)
//v #define PY_DBL_L	14	4	// double Low order 4 bytes
//v #define PY_DBL_H	15	4	// double High order 4 bytes
//v #define PY_ASC	16	1-6	// ASCII char: number = (dlc - 2)
//v
//v Mixed combinations
//v #define PY_U8S32	17	5	// u8 + s32 (e.g. index + reading)
//v #define PY_24bS	18	3,6	// 24b signed (one or two)
//v #define PY_12FFF	19	6	// 1/2flt + full flt
//v #define PY_U16U32	20	6	// u16 + u32

/******************************************************************************
 * int32_t cancmd_paycnvt_to(struct CANRCVBUF* pcan, void* pele, uint32_t id);
 * @brief	: Convert parameter element to CAN msg payload bytes
 * @param	: pcan = pointer in CAN msg
 * @param	: pele = pointer to element in struct
 * @param	: id = parameter id number
 ******************************************************************************/
static uint32_t dlcok(struct CANRCVBUF* pcan)
{
	if (pcan->dlc > 8) return 8;	// From running outside the array 
	if (pcan->dlc < 3) return 0;	// Not enough to be useful
	return (pcan->dlc - 2);		// Account for 1st two bytes used
}

int32_t cancmd_paycnvt_to(struct CANRCVBUF* pcan, void* pele, uint32_t id)
{
	uint8_t*   p8 = ( uint8_t*)pele;
	uint16_t* p16 = (uint16_t*)pele;
	uint32_t* p32 = (uint32_t*)pele;
	uint64_t* p64 = (uint64_t*)pele;
	
	float*  pflt = (float*)pele;
	double* pdbl = (double*)pele;

	uint8_t* ppay = &pcan->cd.uc[0]; // Payload output pointer

	/* Look up type code, given the id. */
	uint_t32 type =  id_v_struct_gettype(id);
	if (type < 0) return -1;

	switch (type)
	case PY_S8:
	case PY_U8:
	case PY_ASC:
		for (i = 0; i < dlcok(pcan); i++)
		{
			*ppay++ = *p8++;
		}
		break;

	case PY_U16:
	case PY_S16:
		*ppay++ = *p16   >> 0;
		*ppay++ = *p16++ >> 8;
		if (dlcok(pcan) < 3) break;
		*ppay++ = *p16 & 0xff;
		*ppay++ = *p16++ >> 8;
		if (dlcok(pcan) < 6) break;
		*ppay++ = *p16   >> 0;
		*ppay++ = *p16++ >> 8;
		break;

	case PY_S32: 
	case PY_U32: 			
		*ppay++ = *p32 >>  0;
		*ppay++ = *p32 >>  8;
		*ppay++ = *p32 >> 16;
		*ppay++ = *p32 >> 24;
		break;

	case PY_S64_L:
	case PY_U64_L:
		*ppay++ = *p64 >>  0;
		*ppay++ = *p64 >>  8;
		*ppay++ = *p64 >> 16;
		*ppay++ = *p64 >> 24;
		break;
		
	case PY_S64_H:
	case PY_U64_H:
		*ppay++ = *p64 >> 32;
		*ppay++ = *p64 >> 40;
		*ppay++ = *p64 >> 48;
		*ppay++ = *p64 >> 56;
		break;

	case PY_FLT:
		*ppay++ = *pflt >>  0;
		*ppay++ = *pflt >>  8;
		*ppay++ = *pflt >> 16;
		*ppay++ = *pfkt >> 24;
		break;
		
	case PY_DBL_L:
		*ppay++ = *pdbl >>  0;
		*ppay++ = *pdbl >>  8;
		*ppay++ = *pdbl >> 16;
		*ppay++ = *pdbk >> 24;
		break;

	case PY_DBL_H:
		*ppay++ = *pdbl >> 32;
		*ppay++ = *pdbl >> 40;
		*ppay++ = *pdbl >> 48;
		*ppay++ = *pdbl >> 56;
		break;

	case PY_12FLT:
		floattopayhalffp((ppay+0), *(pflt+0));
		if (dlcok(pcan) < 3) break;
		floattopayhalffp((ppay+2), *(pflt+1));
		if (dlcok(pcan) < 6) break;
		floattopayhalffp((ppay+4), *(pflt+2));
		break;

	case PY_34flt:
		floattopay3qtrfp((ppay+0), *(pflt+0));
		if (dlcok(pcan) < 4) break;
		floattopay3qtrfp((ppay+3), *(pflt+1));
		break;

	return 0;
}
/******************************************************************************
 * int32_t cancmd_paycnvt_from(struct CANRCVBUF* pcan, void* pele, uint32_t id);
 * @brief	: Convert CAN msg payload bytes from parameter to element in struct
 * @param	: pcan = pointer in CAN msg
 * @param	: pele = pointer to element in struct
 * @param	: id = parameter id number
 * @return	: 0 = OK; -1 = type is not in switch cases
 ******************************************************************************/
int32_t cancmd_paycnvt_from(struct CANRCVBUF* pcan, void* pele, uint32_t id)
{
	uint8_t*   p8 = ( uint8_t*)pele;
	uint16_t* p16 = (uint16_t*)pele;
	uint32_t* p32 = (uint32_t*)pele;
	uint64_t* p64 = (uint64_t*)pele;
	
	float*  pflt = (float*)pele;
	double* pdbl = (double*)pele;

	uint8_t* ppay = &pcan->cd.uc[0]; // Payload input pointer

	/* Look up type code, given the id. */
	uint_t32 type =  id_v_struct_gettype(id);

	switch (type)
	case PY_S8:
	case PY_U8:
	case PY_ASC:
		for (i = 0; i < dlcok(pcan); i++)
		{
			*p8++ = *ppay++;
		}
		break;

	case PY_U16:
	case PY_S16:
		*p16    = (*ppay++ << 0);
		*p16++ |= (*ppay++ << 8);
		if (dlcok(pcan) < 3) break;
		*p16    = (*ppay++ << 0);
		*p16++ |= (*ppay++ << 8);
		if (dlcok(pcan) < 6) break;
		*p16  =  (*ppay++ << 0);
		*p16 |= (*ppay++ << 8);
		break;

	case PY_S32: 
	case PY_U32: 	
		*p32  = (*ppay++ <<  0);		
		*p32 |= (*ppay++ <<  8);	
		*p32 |= (*ppay++ << 16);	
		*p32 |= (*ppay++ << 24);	
		break;

	case PY_S64_L:
	case PY_U64_L:
		*p64  = (*ppay++ <<  0);		
		*p64 |= (*ppay++ <<  8);	
		*p64 |= (*ppay++ << 16);	
		*p64 |= (*ppay++ << 24);
		break;
		
	case PY_S64_H:
	case PY_U64_H:
		*p64  = (*ppay++ << 32);		
		*p64 |= (*ppay++ << 40);	
		*p64 |= (*ppay++ << 48);	
		*p64 |= (*ppay++ << 56);
		break;

	case PY_FLT:
		*pflt  = (*ppay++ <<  0);		
		*pflt |= (*ppay++ <<  8);	
		*pflt |= (*ppay++ << 16);	
		*pflt |= (*ppay++ << 24);	
		break;
		
	case PY_DBL_L:
		*pdbl  = (*ppay++ <<  0);		
		*pdbl |= (*ppay++ <<  8);	
		*pdbl |= (*ppay++ << 16);	
		*pdbl |= (*ppay++ << 24);
		break;

	case PY_DBL_H:
		*pdbl  = (*ppay++ << 32);		
		*pdbl |= (*ppay++ << 40);	
		*pdbl |= (*ppay++ << 48);	
		*pdbl |= (*ppay++ << 56);
		break;

	case PY_12FLT:
		*(pflt+0) = payhalffptofloat((ppay+0));
		if (dlcok(pcan) < 3) break;
		*(pflt+1) = payhalffptofloat((ppay+2));
		if (dlcok(pcan) < 6) break;
		*(pflt+2) = payhalffptofloat((ppay+4));
		break;

	case PY_34flt:
		*(pflt+0) = pay3qtrfptofloat((ppay+0));
		if (dlcok(pcan) < 4) break;
		*(pflt+1) = pay3qtrfptofloat((ppay+3));
		break;

	default: return -1;

	return 0;
}
