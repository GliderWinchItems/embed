/******************************************************************************
* File Name          : common_highflash.c
* Date First Issued  : 10/15/2014
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : Fixed area in high flash for application calibrations
*******************************************************************************/

#include "common_highflash.h"
#include "crc-32.h"

extern u8* __highflashlayout;

static u8* commoncheck(u8* pstart, int size, u32 ver)
{
printf("Xpstart: %08X size: %d ver: %d:: ",(u32)pstart, size, ver);
	/* Compute crc on all but crc at beginning of struct. */
	u32 crc = rc_crc32((pstart+4), (size - sizeof(u32)) );

	/* Compare crc-32's */
	if (crc != *(u32*)pstart)
{printf("crc: %08X %08X\n\r", crc,*(u32*)pstart);USART1_txint_send();
 return 0;	// Return: versions do not match
}

	/* Compare version numbers */
	if (ver != *((u32*)pstart + 1))
{printf("version: %d %d\n\r", ver, *((u32*)pstart + 1) );USART1_txint_send();
 return 0;
}

	return pstart;	// Return pointer to struct (as u8*)
}

/*******************************************************************************
 * struct APPCANID*   gc_hiflash_appcanid(u32 version);
 * struct CRCPROG*    gc_hiflash_crcprog(u32 version);
 * struct BOARDCALIB* gc_hiflash_boardcalib(u32 version);
 * struct APPPARAM*   gc_hiflash_appparam(u32 version);
 * struct HIGHFLASHLAYOUT* gc_hiflash_highflash(u32 version);
 * @brief 	: Check crc and return pointer to struct
 * @param	: version = version number expected
 * @return	: NULL = crc failed or version did not match, otherwise pointer 
*******************************************************************************/
struct APPCANID* gc_hiflash_appcanid(u32 version)
{
	struct HIGHFLASHLAYOUT* p = (struct HIGHFLASHLAYOUT*)&__highflashlayout;
	return (struct APPCANID*)commoncheck( (u8*)&p->appcanid, sizeof(struct APPCANID), version );
};

struct CRCPROG* gc_hiflash_crcprog(u32 version)
{
	struct HIGHFLASHLAYOUT* p = (struct HIGHFLASHLAYOUT*)&__highflashlayout;
	return (struct CRCPROG*)commoncheck((u8*)&p->crcprog, sizeof(struct CRCPROG), version );
};

struct BOARDCALIB* gc_hiflash_boardcalib(u32 version)
{
	struct HIGHFLASHLAYOUT* p = (struct HIGHFLASHLAYOUT*)&__highflashlayout;
	return (struct BOARDCALIB*)commoncheck((u8*)&p->boardcalib, sizeof(struct BOARDCALIB), version );
};

struct APPPARAM* gc_hiflash_appparam(u32 version)
{
	struct HIGHFLASHLAYOUT* p = (struct HIGHFLASHLAYOUT*)&__highflashlayout;
	return (struct APPPARAM*)commoncheck((u8*)&p->appparam, sizeof(struct APPPARAM), version );
};

struct HIGHFLASHLAYOUT* gc_hiflash_highflash(u32 version)
{
	return (struct HIGHFLASHLAYOUT*)commoncheck((u8*)&__highflashlayout, sizeof(struct HIGHFLASHLAYOUT), version );
};


