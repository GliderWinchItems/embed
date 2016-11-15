/******************************************************************************
* File Name          : pay_fIX_cnv.c
* Date First Issued  : 01/15/015
* Board              : little endian
* Description        : Conversion to/from fixed types to payload bytes
*******************************************************************************/
/*
These routines are used to translate a fix number types, e.g. int, unsigned int, to
the CAN msg payload.  

The access to the payload is via a pointer and the there is no restriction on the
memory align boundary.

Endianness is not inlcuded in these routines.  They are based on little endian and
for big endian code will have to be added to deal with the reversed byte order.

*/
#include <stdint.h>
#include <stdio.h>

/******************************************************************************
 * void int32_ttopay(uint8_t *p, int32_t i);
 * @brief 	: Convert int32_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
void int32_ttopay(uint8_t *p, int32_t i)
{	
	*(p+0) = i;
	*(p+1) = i >> 8;
	*(p+2) = i >> 16;
	*(p+3) = i >> 24;
	return;
}
/******************************************************************************
 * int32_t paytoint32_t(uint8_t *p);
 * @brief 	: Convert CAN payload bytes holding a 4 bytes to an int32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
int32_t paytoint32_t(uint8_t *p)
{
	return ( (*(p+3) << 24) | (*(p+2) << 16) | (*(p+1) << 8) | (*p+0)) );
}
/******************************************************************************
 * void uint32_ttopay(uint8_t *p, uint32_t i);
 * @brief 	: Convert uint32_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
void uint32_ttopay(uint8_t *p, uint32_t i)
{	
	*(p+0) = i;
	*(p+1) = i >> 8;
	*(p+2) = i >> 16;
	*(p+3) = i >> 24;
	return;
}
/******************************************************************************
 * uint32_t paytouint32_t(uint8_t *p);
 * @brief 	: Convert CAN payload bytes holding a 4 bytes to a uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: uint32_t
*******************************************************************************/
uint32_t paytouint32_t(uint8_t *p)
{
	return  ((*(p+3) << 24) | (*(p+2) << 16) | (*(p+1) << 8) | (*p+0)) );
}
/******************************************************************************
 * void int16_ttopay(uint8_t *p, int16_t i);
 * @brief 	: Convert int16_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
void int16_ttopay(uint8_t *p, int16_t i)
{	
	*(p+0) = i;
	*(p+1) = i >> 8;
	return;
}/******************************************************************************
 * int paytoint16_t(uint8_t *p);
 * @brief 	: Convert CAN payload bytes holding a 4 bytes to an int16_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
int16_t paytoint16_t(uint8_t *p)
{
	return ( (*(p+1) << 8) | (*p+0)) );
}
/******************************************************************************
 * void uint16_ttopay(uint8_t *p, uint16_t i);
 * @brief 	: Convert int16_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
void uint16_ttopay(uint8_t *p, uint16_t i)
{	
	*(p+0) = i;
	*(p+1) = i >> 8;
	return;
}
/******************************************************************************
 * int paytouint32_t(uint8_t *p);
 * @brief 	: Convert CAN payload bytes holding a 4 bytes to an uint_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
uint16_t paytoint16_t(uint8_t *p)
{
	return ( (*(p+1) << 8) | (*p+0)) );
}
/******************************************************************************
 * void uint32_ttopay3bytes(uint8_t *p, uint32_t ui);
 * @brief 	: Convert uint32_t to three CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: ui = unsigned int to be converted
*******************************************************************************/
void uint32_ttopay3bytes(uint8_t *p, uint32_t ui)
{	
	*(p+0) = ui;
	*(p+1) = ui >> 8;
	*(p+2) = ui >> 16;
	return;
}
/******************************************************************************
 * uint32_t pay3bytestouint32_t(uint8_t *p);
 * @brief 	: Convert CAN payload bytes holding a 3 bytes to an uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
uint32_t pay3bytestouint32_t(uint8_t *p)
{
	return ( (*(p+2) << 16) | (*(p+1) << 8) | (*p+0)) );
}
/******************************************************************************
 * void int32_ttopay3bytes(uint8_t *p, int32_t i);
 * @brief 	: Convert int32_t to three CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = signed int to be converted
*******************************************************************************/
void int32_ttopay3bytes(uint8_t *p, int32_t i)
{	
	*(p+0) = i;
	*(p+1) = i >> 8;
	*(p+2) = i >> 16;
	return;
}
/******************************************************************************
 * int pay3bytestouint32_t(uint8_t *p);
 * @brief 	: Convert CAN payload bytes holding a 3 bytes to an uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
int pay3bytestouint32_t(uint8_t *p)
{
	return ( (int8_t)(*(p+2) << 16) | (*(p+1) << 8) | (*p+0)) );
}


