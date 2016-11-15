/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_cid.h
* Hackeroo           : deh
* Date First Issued  : 08/29/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Extraction of sd card cid fields for version 1 sd cards
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDCARD_CID
#define __SDCARD_CID

/* Usage example:

int csizemult;
...
[As part of initialization of sd card cid is read.]
...
csizemult = sdcard_cid_extract_v1(buf_cid, SDC_C_SIZE_MULT);
...
*/


/* 
(Starting page 75: Simplified_Physcial_Layer_Speccopy.pdf)
   SD Specifications
         Part 1
    Physical Layer
Simplified Specification
       Version 2.00
    September 25, 2006

          VERSION 1
Name                    Field         Width    CID-slice
Manufacturer ID         MID           8        [127:120]
OEM/Application ID      OID           16       [119:104]
Product name            PNM           40       [103:64]
Product revision        PRV           8        [63:56]
Product serial number   PSN           32       [55:24]
reserved                --            4        [23:20]
Manufacturing date      MDT           12       [19:8]
CRC7 checksum           CRC           7        [7:1]
not used, always 1      -             1        [0:0]
                      Table 5-2: The CID Fields
*/

#define SDC_CSD_STRUCTURE      126,2   // 00b           R      [127:126]

#define SDC_CID_MID       126   //  8       [127:120] Manufacturer ID         
#define SDC_CID_OID       104   // 16       [119:104] OEM/Application ID      
#define SDC_CID_PNM        64   // 40       [103:64]  Product name            
#define SDC_CID_PRV        56   //  8       [63:56]   Product revision        
#define SDC_CID_PSN        55   // 32       [55:24]   Product serial number   binary 32b
#define SDC_CID_MDT         8   // 12       [19:8]    Manufacturing date    binary yyyymm (yr - 2000) (month 1= jan)  
#define SDC_CID_CRC         1   //  7       [7:1]     CRC7 checksum           
#define SDC_CID_ALWAYS_1    0   //  1       [0:0]     Always one

/**********************************************************************************/
int sdcard_cid_extract_v1 (char * p, char* cid, unsigned short usFieldBitNumber);
/* @brief	: Extract the bit field into an easy-to-use int from the CSD data (sd v1)
 * @param	: ASCII string return.  Max size including terminator 6 bytes.
 * @param	: Pointer to CSD record (16 bytes)
 * @param	: Bit number of the field of the low order bit
 * @return	: negative = field was ascii; postivie = value in field, 65536 = usFieldBitNumber
**********************************************************************************/


#endif 

