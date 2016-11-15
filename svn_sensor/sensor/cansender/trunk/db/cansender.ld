OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
/* Linker script for STM32F103VET6, 512K flash, 64K RAM */

/* Define memory regions. */
MEMORY
{

/* FLASHPx: Areas for arrays with parameters */
	FLASHH   (RX)  : ORIGIN = 0x08000000 + 512K - 0x0800, LENGTH = 0x800
	FLASHP1  (RX)  : ORIGIN = 0x08000000 + 512K - 0x0800 - 0x0800, LENGTH = 0x0800
	FLASHP2  (RX)  : ORIGIN = 0x08000000 + 512K - 0x0800 - 0x0800 - 0x0800, LENGTH = 0x0800
	FLASHP3  (RX)  : ORIGIN = 0x08000000 + 512K - 0x0800 - 0x0800 - 0x0800 - 0x0800, LENGTH = 0x0800
	FLASHP4  (RX)  : ORIGIN = 0x08000000 + 512K - 0x0800 - 0x0800 - 0x0800 - 0x0800 - 0x0800, LENGTH = 0x0800
}


/* Let programs know what they have to work with. */
__sizeof_flashh  = 0x0800;
__sizeof_flashp1 = 0x0800;
__sizeof_flashp2 = 0x0800;
__sizeof_flashp3 = 0x0800;
__sizeof_flashp4 = 0x0800;

SECTIONS
{


/* A flash area for storing parameters. */

	.flashh :
	{
		__highflashlayout = .;
		*(.appparam)
		. = ALIGN(4);

	} 	>FLASHH

	.flashp1 :
	{
		__paramflash1 = .;
		*(.appparam1)

	} 	>FLASHP1
	
	.flashp2 :
	{
		__paramflash2 = .;
		*(.appparam2)
		. = ALIGN(4);

	} 	>FLASHP2
	
	.flashp3 :
	{
		__paramflash3 = .;
		*(.appparam3)
		. = ALIGN(4);

	} 	>FLASHP3
	
	.flashp4 :
	{
		__paramflash4 = .;
		*(.appparam4)
		. = ALIGN(4);

	} 	>FLASHP4
	
	

}



