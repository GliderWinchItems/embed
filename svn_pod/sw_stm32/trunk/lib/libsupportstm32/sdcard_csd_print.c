/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_csd_print.c
* Hacker             : deh
* Date First Issued  : 08/28/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : List CSD fields
*******************************************************************************/
#include "sdcard_csd.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "sdcard_ll.h"
#include "libmiscstm32/printf.h"

/* Shameless hack to make it work on POD without changing it for Olimex */
char sdcard_csd_print_POD	; // 0 = USART2; 1 = USART1

static void select_usart(void)
{
	if (sdcard_csd_print_POD == 1)
		USART1_txint_send();
	else
		USART2_txint_send();
	return;
}


/*****************************************************************************************/
/* Convert list out CSD 			                                         */
/*****************************************************************************************/
static void sdcard_csd_dump(char *p)
{
	int i;
	printf ("\n\rCSD "); 	select_usart();
	for (i = 0; i < SDC_CSD_SIZE; i++)
	{
		printf ("%02x ",*p++);
	}
	printf ("\n\r");	select_usart();
	return;
}
/*****************************************************************************************
* void sdcard_csd_print(char* p);
* @brief	: printf the CSD fields 
* @param	: pointer to CSD byte array (16 bytes)
*****************************************************************************************/

void sdcard_csd_print(char* p)
{
	int x;
	sdcard_csd_dump(p);

	x = sdcard_csd_extract_v1 (p,SDC_CSD_STRUCTURE );
	char *p1 = "[127:126]";	
	printf ("SDC_CSD_STRUCTURE      %s = %6u %6x // CSD V",p1,x,x); select_usart();
	if((*p & 0xc0) == 0)
		printf("1.0 (SDSC");
	else
		printf("2.0 (SDHC/SDXC");
	printf(" card)\n\r"); select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_TAAC );
	p1 = "[119:112]";	
	printf ("SDC_TAAC               %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_NSAC );
	p1 = "[111:104]";	
	printf ("SDC_NSAC               %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_TRAN_SPEED );
	p1 = " [103:96]";	
	printf ("SDC_TRAN_SPEED         %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_CCC );
	p1 = "  [95:84]";	
	printf ("SDC_CCC                %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_READ_BL_LEN );
	p1 = "  [83:80]";	
	printf ("SDC_READ_BL_LEN        %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_READ_BL_PARTIAL );
	p1 = "  [95:84]";	
	printf ("SDC_READ_BL_PARTIAL    %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_WRITE_BLK_MISALIGN );
	p1 = "  [78:78]";	
	printf ("SDC_WRITE_BLK_MISALIGN %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_READ_BLK_MISALIGN );
	p1 = "  [77:77]";	
	printf ("SDC_READ_BLK_MISALIGN  %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_DSR_IMP );
	p1 = "  [76:76]";	
	printf ("SDC_DSR_IMP            %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	if((*p & 0xc0) == 0)		/* CSD V1.0 (SDSC card) */
	{
		x = sdcard_csd_extract_v1 (p,SDC_C_SIZE );
		p1 = "  [73:62]";	
		printf ("SDC_C_SIZE             %s = %6u %6x\n\r",p1,x,x); 	select_usart();
	
		x = sdcard_csd_extract_v1 (p,SDC_VDD_R_CURR_MIN );
		p1 = "  [61:59]";	
		printf ("SDC_VDD_R_CURR_MIN     %s = %6u %6x\n\r",p1,x,x); 	select_usart();
	
		x = sdcard_csd_extract_v1 (p,SDC_VDD_R_CURR_MAX);
		p1 = "  [58:56]";	
		printf ("SDC_VDD_R_CURR_MAX     %s = %6u %6x\n\r",p1,x,x); 	select_usart();
	
		x = sdcard_csd_extract_v1 (p,SDC_VDD_W_CURR_MIN);
		p1 = "  [55:53]";	
		printf ("SDC_VDD_W_CURR_MIN     %s = %6u %6x\n\r",p1,x,x); 	select_usart();
	
		x = sdcard_csd_extract_v1 (p,SDC_VDD_W_CURR_MAX);
		p1 = "  [52:50]";	
		printf ("SDC_VDD_W_CURR_MAX     %s = %6u %6x\n\r",p1,x,x); 	select_usart();
	
		x = sdcard_csd_extract_v1 (p,SDC_C_SIZE_MULT);
		p1 = "  [49:47]";	
		printf ("SDC_C_SIZE_MULT        %s = %6u %6x\n\r",p1,x,x); 	select_usart();
	}
	else					/* CSD V2.0 (SDHC/SDXC card) */
	{
		x = sdcard_csd_extract_v1 (p,SDC2_C_SIZE );
		p1 = "  [69:48]";	
		printf ("SDC2_C_SIZE            %s = %6u %6x\n\r",p1,x,x); 	select_usart();
	}
	
	x = sdcard_csd_extract_v1 (p,SDC_ERASE_BLK_EN);
	p1 = "  [46:46]";	
	printf ("SDC_ERASE_BLK_EN       %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_SECTOR_SIZE);
	p1 = "  [45:39]";	
	printf ("SDC_SECTOR_SIZE        %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_WP_GRP_SIZE );
	p1 = "  [38:32]";	
	printf ("SDC_WP_GRP_SIZE        %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_WP_GRP_ENABLE );
	p1 = "  [31:31]";	
	printf ("SDC_WP_GRP_ENABLE      %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_R2W_FACTOR );
	p1 = "  [28:26]";	
	printf ("SDC_R2W_FACTOR         %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_WRITE_BL_LEN );
	p1 = "  [25:22]";	
	printf ("SDC_WRITE_BL_LEN       %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_WRITE_BL_PARTIAL );
	p1 = "  [21:21]";	
	printf ("SDC_WRITE_BL_PARTIAL   %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_FILE_FORMAT_GRP );
	p1 = "  [15:15]";	
	printf ("SDC_FILE_FORMAT_GRP    %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_COPY );
	p1 = "  [14:14]";	
	printf ("SDC_COPY               %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_PERM_WRITE_PROTECT );
	p1 = "  [13:13]";	
	printf ("SDC_PERM_WRITE_PROTECT %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_TMP_WRITE_PROTECT );
	p1 = "  [12:12]";	
	printf ("SDC_TMP_WRITE_PROTECT  %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_FILE_FORMAT );
	p1 = "  [11:10]";	
	printf ("SDC_FILE_FORMAT        %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_CRC );
	p1 = "    [7:1]";	
	printf ("SDC_CRC                %s = %6u %6x\n\r",p1,x,x); 	select_usart();

	x = sdcard_csd_extract_v1 (p,SDC_ALWAYS_1 );
	p1 = "    [0:0]";	
	printf ("SDC_ALWAYS_1           %s = %6u %6x\n\r",p1,x,x); 	select_usart();



	return;



}
