OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
/* Linker script for STM32F103VET6, 512K flash, 64K RAM */

ENTRY (Reset_Handler);

/* Define memory regions. */
MEMORY
{
	RAM      (RWX) : ORIGIN = 0x20000000, LENGTH = 64K
	FLASH    (RX)  : ORIGIN = 0x08005000, LENGTH = 512K - 0x5000 - 0x0800 - 0x0800 - 0x0800 - 0x0800 - 0x0800
/* FLASHH: Area for CAN IDs used by "functions" in this app. */
	FLASHH	 (RX)  : ORIGIN = 0x08000000 + 512K - 0x800, LENGTH = 0x800
/* FLASHPx: Areas for arrays with parameters */
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

/* Address of application jump address */
__appjump    = 0x08005000;
__appoffset  = 0x08005000;

_MYHEAP_SIZE = 1024;
__MSP = 0x20000000 + 20K - 2048;



SECTIONS
{
	.flash	:
	{
		*(.vectors)
                *(.text .text.* .gnu.linkonce.t.*)
                *(.plt)
                *(.gnu.warning)
                *(.glue_7t) 
                *(.glue_7) 
                *(.vfp11_veneer)
                *(.ARM.extab* .gnu.linkonce.armextab.*)
                *(.gcc_except_table)
		*(.v4_bx )
		*(.rodata.str1.*)
		*(.rodata)
		. = ALIGN(4);

	} 	>FLASH

	.flashh :
	{
		__highflashlayout = .;
		*(.appparam)
		. = ALIGN(4);

	} 	>FLASHH

/* A flash area for storing parameters. */
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
	
.ARM.extab :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH

   /* .ARM.exidx is sorted, so has to go in its own output section.*/
   .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
		__data_image = .;
	  	PROVIDE( __data_image = . );
    } >FLASH


	.data	: ALIGN (4)
	{
		__data_section_start = .;
                _sdata = .;        /* global needed for startup_stm3210x_md_vl.S */
		PROVIDE ( __data_section_start = .); 
                *(.cs3.region-head.ram)
                KEEP(*(.jcr))
                *(.got.plt) *(.got)
                *(.shdata)
                *(.data .data.* .gnu.linkonce.d.*)
                _edata = .;        /* global needed for startup_stm3210x_md_vl.S */
		__data_image_end = .;
	  	PROVIDE( __data_image_end = . );
		_end_data = .;
	. = ALIGN(4);
	} >RAM AT>FLASH



	.ram	:
  	{
    		PROVIDE (_start_of_bss = .);
                _sbss = .;         /* global needed for startup_stm3210x_md_vl.S */
		__bss_start__ = .;
    		*(.bss)    
		*(.bss*)
                *(.shbss)
                *(.bss .bss.* .gnu.linkonce.b.*)
                *(COMMON)
                . = ALIGN(4);
                *(.ram.b .bss.ram)
                . = ALIGN (4);
                 _ebss = .;        /* global needed for startup_stm3210x_md_vl.S */
    		_end_of_bss = .;
		__bss_end__ = .;
 	        _end = .;
                __end = .;
		PROVIDE (_end_of_bss = .);

  	}  >RAM 
}



