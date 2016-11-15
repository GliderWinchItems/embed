/* fcscax25.s subroutine */
#ifndef _CRC16
#define _CRC16


/***************************************************** 
;* Calculate HDLC type of crc--
;*    polynomial for AX.25 spec: 1 + x^5 + x^12 + x^16
;***************************************************** */
unsigned int CRC16c(unsigned int crc, unsigned char *ptr, unsigned int ct);/* c version */
unsigned int CRC16s(unsigned int crc, unsigned char *ptr, unsigned int ct);/* asm version */
/*  Where:
crc	= initial value (0x0000 for SD card)
*ptr 	= pointer to byte array with input data
ct	= number of bytes in array
Return: computed crc
*/


#endif
