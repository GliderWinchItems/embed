/* File: crc_ccitt.h -- Interface for crc_ccitt.
 */
#ifndef __CRC_CCITT_H
#define __CRC_CCITT_H


/***************************************************** 
 * Calculate HDLC type of crc--
 *    polynomial for AX.25 spec: 1 + x^5 + x^12 + x^16
******************************************************/

unsigned short crc_ccitt(unsigned short crc, unsigned char *ptr, unsigned int ct);

/* Where:
 * 	crc	= initial value (0x0000 for SD card)
 * 	ptr = pointer to byte array with input data
 * 	ct	= number of bytes in array
 * Return: computed crc
*/


#endif
