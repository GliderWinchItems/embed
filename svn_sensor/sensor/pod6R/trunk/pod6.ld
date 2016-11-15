OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
/* Linker script for STM32F103VCT6, 128K flash, 20K RAM */

ENTRY (Reset_Handler);

/* Define memory regions. */
MEMORY
{
	RAM      (RWX) : ORIGIN = 0x20000000, LENGTH = 20K
	FLASH    (RX)  : ORIGIN = 0x08005000, LENGTH = 256K-0x800-0x5000
/* FLASHH: One 2048 byte block in high flash for app calibrations, etc. */
	FLASHH	 (RX)  : ORIGIN = 0x08000000 + 256K - 0x800, LENGTH = 0x800
}

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
		*(.ctbltext)
		*(.ctbldata)
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
		__appparam = .;
		*(.appparam)
		. = ALIGN(4);

	} 	>FLASHH


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
		*(.ramvector)
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

	.flashe	:
	{
		__flashend = .;
		*(.flashend)
		. = ALIGN(4);
	} >FLASH

	.ram	:
  	{
    		PROVIDE (_start_of_bss = .);
                _sbss = .;         /* global needed for startup_stm3210x_md_vl.S */
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
 	        _end = .;
                __end = .;
		PROVIDE (_end_of_bss = .);

  	}  >RAM 
}



