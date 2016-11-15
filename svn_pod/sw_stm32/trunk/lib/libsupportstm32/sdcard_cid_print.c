/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_cd_print.c
* Hacker             : deh
* Date First Issued  : 08/29/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : List CID fields
*******************************************************************************/
#include "sdcard_cid.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "sdcard_ll.h"
#include "libmiscstm32/printf.h"

/* Shameless hack to make it work on POD without changing it for Olimex */
char sdcard_cid_print_POD	; // 0 = USART2; 1 = USART1

static void select_usart(void)
{
	if (sdcard_cid_print_POD == 1)
		USART1_txint_send();
	else
		USART2_txint_send();
	return;
}


/*****************************************************************************************/
/* Convert list out CID 			                                         */
/*****************************************************************************************/
static void sdcard_cid_dump(char *p)
{
	int i;
	printf ("\n\rCID "); 	select_usart();
	for (i = 0; i < SDC_CID_SIZE; i++)
	{
		printf ("%02x ",*p++);
	}
	printf ("\n\r");  	select_usart();
	return;
}
/*****************************************************************************************
* void sdcard_cid_print(char* p);
* @brief	: printf the CID fields 
* @param	: pointer to CID byte array (16 bytes)
*****************************************************************************************/

void sdcard_cid_print(char* p)
{
	int x;
	char *p1;
	char c[8];			// Holds ASCII char returns
	sdcard_cid_dump(p);		// Hex dump of the CID line

	x = sdcard_cid_extract_v1 (c,p,SDC_CID_MID  );
	p1 = "[127:120]";	
	printf ("SDC_CID_MID       %s = %6u %6x",p1,x,x);
	printf ("   : Manufacturer ID \n\r");			 	select_usart();

	x = sdcard_cid_extract_v1 (c,p,SDC_CID_OID );
	p1 = "[119:104]";	
	printf ("SDC_CID_OID       %s =      %s ",p1,c);
	printf ("        : OEM/Application ID \n\r");			 	select_usart();

	x = sdcard_cid_extract_v1 (c,p,SDC_CID_PNM );
	p1 = "[103:64]";	
	printf ("SDC_CID_PNM       %s  =      %s ",p1,c);
	printf ("     : Product name\n\r");			 	select_usart();

	x = sdcard_cid_extract_v1 (c,p,SDC_CID_PRV  );
	c[0] = ( ((x >> 4) & 0x0f) | '0');
	c[1] = '.';
	c[2] = (  (x & 0x0f) | '0');
	c[3] = 0;
	p1 = "[63:56]";	
	printf ("SDC_CID_PRV       %s  =       %s ",p1,c);
	printf ("       : Product revision \n\r");			 	select_usart();

	x = sdcard_cid_extract_v1 (c,p,SDC_CID_PSN  );
	p1 = "[55:24]";	
	printf ("SDC_CID_PSN       %s =%9u %08x",p1,x,x);
	printf (" : Product serial number\n\r");			 	select_usart();

	x = sdcard_cid_extract_v1 (c,p,SDC_CID_MDT  );
	p1 = "[19:8]";	
	printf ("SDC_CID_MDT       %s    = %6u     %4u",p1,((x>>4) & 0xff)+2000,(x & 0xf));
	printf (" : Manufacturing date (yr mo)\n\r");			 	select_usart();


	x = sdcard_cid_extract_v1 (c,p,SDC_CID_CRC  );
	p1 = "[7:1]";	
	printf ("SDC_CID_CRC       %s     = %6u   %6x",p1,x,x);
	printf (" : CRC7 checksum\n\r");			 	select_usart();


	x = sdcard_cid_extract_v1 (c,p,SDC_CID_ALWAYS_1  );
	p1 = "[0:0]";	
	printf ("SDC_CID_ALWAYS_1  %s     = %6u   %6x",p1,x,x);
	printf (" : Always one\n\r");			 	select_usart();


	return;



}
