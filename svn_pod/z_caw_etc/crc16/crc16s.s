;**************************************************************************************************** */
; crc16s.s Compute CRC16 for SD card
; 09/12/2007
; The polynomial to generate the table = 0x1021
; See crc16TableGenerator.c for code that generated the table (n C format)
; The code implements the code in crc16.c
; This routine is 3.36 times faster than the C code
; Times for computing a crc on a block of 512 bytes (24MHz bus)
;   Assembly routine:  0.652 ms per block
;          C routine:  2.19  ms per block
;**************************************************************************************************** */
; unsigned int CRC16s(unsigned int crc, unsigned char *ptr, unsigned int ct)
;/*  Where:
;crc	= initial value (0x0000 for SD card)
;*ptr 	= pointer to byte array with input data
;ct	= number of bytes in array
;Return: computed crc
;*/
;	int i;
;
;        for(i = 0; i < ct; i++) ;
;	{
;            crc = (unsigned int)((crc << 8) ^ table[((crc >> 8) ^ (0xff & *(ptr+i)))]);
;        }
;        return crc;
;
;}
;*
	.global      CRC16s
	.section	.text
;*
;*  Stack layout:
;*               Offset for X
;* *** Arguments passed, plus ret address from jsr/bsr branch ***
;*   ct          +11     
;*   ct          +10     ; Count to do
;*   cp  L       +9    
;*   cp  H       +8      ; Address of input chars 
;*   ret L       +7      
;*   ret H       +6      ; Subroutine return address
;* *** Saved register space **********************************
;*   reg X L     +5      
;*   reg X H     +4      ; Saved X reg
;*   reg Y L     +3      
;*   reg Y H     +2      ; Saved Y reg
;* *** local variables space *********************************
;*   crc L       +1      
;*   crc H       +0      ; Crc being built
;*
;* Offsets to be added to X reg to access stack'd vars:
ct      =     10      ; First argument
cp      =     8       ; Add of buffer
crc     =     0       ; Crc being built
;* C code for computation 
; crc = (unsigned int)((crc << 8) ^ table[((crc >> 8) ^ (0xff & *(ptr+i)))]);
 
;* >>> Begin here >>>
CRC16s:
;* Save regs (as a matter of safety)
     pshy              ; Save X & Y.      
     pshx
;*  Setup local variables on stack.
     std     2,-sp     ; Place crc on stack
     ldy     cp,sp     ; Y pts to data buffer
     ldx     ct,sp     ; X = loop ct
crc_a:
     ldab    crc,sp    ; Get high ord byte of crc into low ord of D register 
     eorb    1,y+      ; (crc ^ *cp++) and advance Y ptr
     clra	       ; 
     asld	       ; *2 for tbl of integers
     pshx	       ; Save loop ct
     ldx     #crctable ;
     ldd     d,x       ; Indexed load of table word
     pulx              ; Restore loop ct
     eora    crc+1,sp  ; (crc<<8) ^ crctab...
     std     crc,sp    ; Save new crc
     dbne     x,crc_a     ;   br, if more to do.
;* Get ready for return     
     ldd  2,sp+        ; Return computed crc and clear it from stack
;    eora #0xff        ; crc ^= crc;
;    eorb #0xff        ;
     pulx              ; Restore X to entry value.
     puly              ; Restore Y to entry value.
     rts               ; Return.
;**************************************************************************************************** */
crctable:
 dc.w  0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7
 dc.w  0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef
 dc.w  0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6
 dc.w  0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de
 dc.w  0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485
 dc.w  0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d
 dc.w  0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4
 dc.w  0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc
 dc.w  0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823
 dc.w  0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b
 dc.w  0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12
 dc.w  0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a
 dc.w  0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41
 dc.w  0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49
 dc.w  0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70
 dc.w  0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78
 dc.w  0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f
 dc.w  0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067
 dc.w  0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e
 dc.w  0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256
 dc.w  0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d
 dc.w  0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405
 dc.w  0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c
 dc.w  0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634
 dc.w  0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab
 dc.w  0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3
 dc.w  0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a
 dc.w  0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92
 dc.w  0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9
 dc.w  0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1
 dc.w  0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8
 dc.w  0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0

	.end
