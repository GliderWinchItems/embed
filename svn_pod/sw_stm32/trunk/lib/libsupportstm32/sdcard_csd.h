/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_csd.h
* Hackeroo           : deh
* Date First Issued  : 08/28/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Extraction of sd card csd fields for version 1 sd cards
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDCARD_CSD
#define __SDCARD_CSD

/* Usage example:

int csizemult;
...
[As part of initialization of sd card csd is read.]
...
csizemult = sdcard_csd_extract_v1(buf_csd, SDC_C_SIZE_MULT);
...
*/


/* 
(Starting page 78: Simplified_Physcial_Layer_Speccopy.pdf)
   SD Specifications
         Part 1
    Physical Layer
Simplified Specification
       Version 2.00
    September 25, 2006

          VERSION 1                                                            Cell
Name                             Field              Width Value                CSD-slice
                                                                        Type
CSD structure                    CSD_STRUCTURE      2     00b           R      [127:126]
reserved                         -                  6     00 0000b      R      [125:120]
data read access-time-1          TAAC               8     xxh           R      [119:112]
data read access-time-2 in CLK   NSAC               8     xxh           R      [111:104]
cycles (NSAC*100)
max. data transfer rate          TRAN_SPEED         8     32h or 5Ah    R      [103:96]
card command classes             CCC                12    01x110110101b R      [95:84]
max. read data block length      READ_BL_LEN        4     xh            R      [83:80]
partial blocks for read allowed  READ_BL_PARTIAL    1     1b            R      [79:79]
write block misalignment         WRITE_BLK_MISALIGN 1     xb            R      [78:78]
read block misalignment          READ_BLK_MISALIGN  1     xb            R      [77:77]
DSR implemented                  DSR_IMP            1     xb            R      [76:76]
reserved                         -                  2     00b           R      [75:74]
device size                      C_SIZE             12    xxxh          R      [73:62]
max. read current @VDD min       VDD_R_CURR_MIN     3     xxxb          R      [61:59]
max. read current @VDD max       VDD_R_CURR_MAX     3     xxxb          R      [58:56]
max. write current @VDD min      VDD_W_CURR_MIN     3     xxxb          R      [55:53]
max. write current @VDD max      VDD_W_CURR_MAX     3     xxxb          R      [52:50]
device size multiplier           C_SIZE_MULT        3     xxxb          R      [49:47]
erase single block enable        ERASE_BLK_EN       1     xb            R      [46:46]
erase sector size                SECTOR_SIZE        7     xxxxxxxb      R      [45:39]
write protect group size         WP_GRP_SIZE        7     xxxxxxxb      R      [38:32]
write protect group enable       WP_GRP_ENABLE      1     xb            R      [31:31]
reserved (Do not use)                               2     00b           R      [30:29]
write speed factor               R2W_FACTOR         3     xxxb          R      [28:26]
max. write data block length     WRITE_BL_LEN       4     xxxxb         R      [25:22]
partial blocks for write allowed WRITE_BL_PARTIAL   1     xb            R      [21:21]
reserved                         -                  5     00000b        R      [20:16]
File format group                FILE_FORMAT_GRP    1     xb            R/W(1) [15:15]
copy flag (OTP)                  COPY               1     xb            R/W(1) [14:14]
permanent write protection       PERM_WRITE_PROTECT 1     xb            R/W(1) [13:13]
temporary write protection       TMP_WRITE_PROTECT 1      xb            R/W    [12:12]
File format                      FILE_FORMAT        2     xxb           R/W(1) [11:10]
reserved                                            2     00b           R/W    [9:8]
CRC                              CRC                7     xxxxxxxb      R/W    [7:1]
not used, always’1’              -                  1     1b            -      [0:0]
*/

// The following are for both V1.0 or V2.0 CSD structure
#define SDC_CSD_STRUCTURE      126,2   // 00b for V1.0  R      [127:126]
                            		   // 01b for V2.0
#define SDC_TAAC               112,8   // xxh           R      [119:112]
#define SDC_NSAC               104,8   // xxh           R      [111:104]
#define SDC_TRAN_SPEED         96,8    // 32h or 5Ah    R      [103:96]
#define SDC_CCC                84,12   // 01x110110101b R      [95:84]
#define SDC_READ_BL_LEN        80,4    // xh            R      [83:80]
#define SDC_READ_BL_PARTIAL    79,1    // 1b            R      [79:79]
#define SDC_WRITE_BLK_MISALIGN 78,1    // xb            R      [78:78]
#define SDC_READ_BLK_MISALIGN  77,1    // xb            R      [77:77]
#define SDC_DSR_IMP            76,1    // xb            R      [76:76]

// The following are for V1.0 only
#define SDC_C_SIZE             62,12   // xxxh          R      [73:62]
#define SDC_VDD_R_CURR_MIN     59,3    // xxxb          R      [61:59]
#define SDC_VDD_R_CURR_MAX     56,3    // xxxb          R      [58:56]
#define SDC_VDD_W_CURR_MIN     53,3    // xxxb          R      [55:53]
#define SDC_VDD_W_CURR_MAX     50,3    // xxxb          R      [52:50]
#define SDC_C_SIZE_MULT        47,3    // xxxb          R      [49:47]

// The following are for V2.0 only
#define	SDC2_C_SIZE			   48,22   // xxxxxxxxh     R      [69,48]

// The following are for both V1.0 or V2.0 CSD structure
#define SDC_ERASE_BLK_EN       46,1    // xb            R      [46:46]
#define SDC_SECTOR_SIZE        39,7    // xxxxxxxb      R      [45:39]
#define SDC_WP_GRP_SIZE        32,7    // xxxxxxxb      R      [38:32]
#define SDC_WP_GRP_ENABLE      31,1    // xb            R      [31:31]
#define SDC_R2W_FACTOR         26,3    // xxxb          R      [28:26]
#define SDC_WRITE_BL_LEN       22,4    // xxxxb         R      [25:22]
#define SDC_WRITE_BL_PARTIAL   21,1    // xb            R      [21:21]
#define SDC_FILE_FORMAT_GRP    15,1    // xb            R/W(1) [15:15]
#define SDC_COPY               14,1    // xb            R/W(1) [14:14]
#define SDC_PERM_WRITE_PROTECT 13,1    // xb            R/W(1) [13:13]
#define SDC_TMP_WRITE_PROTECT  12,1    // xb            R/W    [12:12]
#define SDC_FILE_FORMAT        10,2    // xxb           R/W(1) [11:10]
#define SDC_CRC                 1,7    // xxxxxxxb      R/W    [7:1]
#define SDC_ALWAYS_1            0,1    //                      [0:0]



/**********************************************************************************/
int sdcard_csd_extract_v1 (char* csd, unsigned short usFieldBitNumber, unsigned short usNumBits);
/* @brief	: Extract the bit field into an easy-to-use int from the CSD data (sd v1)
 * @param	: Pointer to CSD record (16 bytes)
 * @param	: Bit number of the field of the low order bit
 * @param	: Number of bits
 * @return	: value in the field
**********************************************************************************/
unsigned int sdcard_csd_memory_size (char* csd);
/* @brief	: Compute memory size
 * @param	: Pointer to CSD record (16 bytes)
 * @return	: Card size (number of blocks)
**********************************************************************************/
unsigned int sdcard_csd_block_size (char* csd);
/* @brief	: Compute block size
 * @param	: Pointer to CSD record (16 bytes)
 * @return	: Block size (in bytes)
**********************************************************************************/



#endif 

