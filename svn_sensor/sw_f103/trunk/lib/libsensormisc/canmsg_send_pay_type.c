/******************************************************************************
* File Name          : canmsg_send_pay_type.c
* Date First Issued  : 05/13/1025
* Board              : F103
* Description        : Send payload by payload type
*******************************************************************************/
#include <stdarg.h>
#include "pay_type_cnvt.h"

#include "../../../../svn_common/trunk/db/gen_db.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "canwinch_pod_common_systick2048.h"

/* If any of these in 'can_db.h' change, then the compiler will give a warning. */
#define  FF                      1         //  [0]-[3]: Full Float                            
#define  FF_FF                   2         //  [0]-[3]: Full Float; [4]-[7]: Full Float       
#define  U32                     3         //  [0]-[3]: uint32_t                              
#define  U32_U32                 4         //  [0]-[3]: uint32_t; [4]-[7]: uint32_t           
#define  U8_U32                  5         //  [0]: uint8_t; [1]-[4]: uint32_t                
#define  S32                     6         //  [0]-[3]: int32_t                               
#define  S32_S32                 7         //  [0]-[3]: int32_t; [4]-[7]: int32_t             
#define  U8_S32                  8         //  [0]: int8_t; [4]-[7]: int32_t                  
#define  HF                      9         //  [0]-[1]: Half-Float                            
#define  F34F                    10        //  [0]-[2]: 3/4-Float                             
#define  xFF                     11        //  [1]-[4]: Full-Float, first   byte  skipped     
#define  xxFF                    12        //  [1]-[4]: Full-Float, first 2 bytes skipped     
#define  xxU32                   13        //  [1]-[4]: uint32_t, first 2 bytes skipped       
#define  xxS32                   14        //  [1]-[4]: int32_t, first 2 bytes skipped        
#define  u8_u8_U32               15        //  [0]:[1]:[2]-[5]: uint8_t,uint8_t,uint32_t,     
#define  u8_u8_S32               16        //  [0]:[1]:[2]-[5]: uint8_t,uint8_t, int32_t,     
#define  u8_u8_FF                17        //  [0]:[1]:[2]-[5]: uint8_t,uint8_t, Full Float, 

#define CANMSGPAYMAXRETRYCT	8	// Limit of TERR retries 
#define CANMSGPAYBITS		0	// SoftNART and other bits

/******************************************************************************
 * int canmsg_send_pay_type_FF(struct CANRCVBUF* p, float f);
 * @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: f = float to be converted
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_FF(struct CANRCVBUF* p, float f)
{
	p->dlc = 4;	// Size of payload
	pay_type_cnvt_FloattoPay_FF(&p->cd.uc[0], f);
	return can_msg_put_ext_sys(p, CANMSGPAYMAXRETRYCT, CANMSGPAYBITS);
}
/******************************************************************************
 * int canmsg_send_pay_type_U32_U32toU32_U32(struct CANRCVBUF* p, uint32_t ui1, uint32_t ui2);
 * @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: ui1 = [0]-[3]
 * @param	: ui2 = [4]-[7]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_U32_U32toU32_U32(struct CANRCVBUF* p, uint32_t ui1, uint32_t ui2)
{
	p->dlc = 8;	// Size of payload
	pay_type_cnvt_U32_U32toPay_U32_U32(&p->cd.uc[0], ui1, ui2);
	return can_msg_put_ext_sys(p, CANMSGPAYMAXRETRYCT, CANMSGPAYBITS);
}
/******************************************************************************
 * int canmsg_send_pay_type_S32_S32toS32_S32(struct CANRCVBUF* p, int32_t i1, int32_t i2);
 * @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: i1 = [0]-[3]
 * @param	: i2 = [4]-[7]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_S32_S32toS32_S32(struct CANRCVBUF* p, int32_t i1, int32_t i2)
{
	p->dlc = 8;	// Size of payload
	pay_type_cnvt_S32_S32toPay_S32_S32(&p->cd.uc[0], i1, i2);
	return can_msg_put_ext_sys(p, CANMSGPAYMAXRETRYCT, CANMSGPAYBITS);
}
/******************************************************************************
 * int canmsg_send_pay_type_U16toU16(struct CANRCVBUF* p, uint16_t us);
 * @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: us = [0]-[1]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_U16toU16(struct CANRCVBUF* p, uint16_t us)
{
	p->dlc = 2;	// Size of payload
	pay_type_cnvt_U16toPay_U16(&p->cd.uc[0], us);
	return can_msg_put_ext_sys(p, CANMSGPAYMAXRETRYCT, CANMSGPAYBITS);
}
/******************************************************************************
 * int canmsg_send_pay_type_S16toS16(struct CANRCVBUF* p, int16_t ss);
 * @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: ss = [0]-[1]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_S16toS16(struct CANRCVBUF* p, int16_t ss)
{
	p->dlc = 2;	// Size of payload
	pay_type_cnvt_U16toPay_U16(&p->cd.uc[0], (uint16_t)ss);
	return can_msg_put_ext_sys(p, CANMSGPAYMAXRETRYCT, CANMSGPAYBITS);
}
/******************************************************************************
 * int canmsg_send_pay_type_U32toU32(struct CANRCVBUF* p, uint32_t ui);
 * @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: UI = [0]-[3]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_U32toU32(struct CANRCVBUF* p, uint32_t ui)
{
	p->dlc = 4;	// Size of payload
	pay_type_cnvt_U32toPay_U32(&p->cd.uc[0], ui);
	return can_msg_put_ext_sys(p, CANMSGPAYMAXRETRYCT, CANMSGPAYBITS);
}

