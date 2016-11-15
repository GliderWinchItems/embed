OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
/* Linker script for STM32F103VCT6, 128K flash, 20K RAM */

ENTRY (Reset_Handler);

/* Define memory regions. */
MEMORY
{
	RAM      (RWX) : ORIGIN = 0x20000000, LENGTH = 64K
	FLASH    (RX)  : ORIGIN = 0x08000000, LENGTH = 256K
}

_MYHEAP_SIZE = 8192;

SECTIONS
{
	.ram	:
  	{
    		PROVIDE (_start_of_bss = .);
    		*(.bss)    	
    		_end_of_bss = .;
		PROVIDE (_end_of_bss = .);

  	}  >RAM 
	
	.flash	:
	{
		*(.vectors)
		*(.text)
		*(.glue_7)
		*(.vfp11_veneer)
		*(.v4_bx )
		*(.rodata.str1.*)
		*(.rodata)
		. = ALIGN(2);
		__data_image = .;
	  	PROVIDE( __data_image = . );
	} 	>FLASH
	

  .ARM.extab   : { *(.ARM.extab*) }
   __exidx_start = .;
  .ARM.exidx   : { *(.ARM.exidx*) }
   __exidx_end = .;


	.data	: ALIGN (2)
	{
		__data_section_start = .;
		. = ALIGN(2);
		PROVIDE ( __data_section_start = .); 
		*(.data .data.*)
		. = ALIGN(2);
		__data_image_end = .;
	  	PROVIDE( __data_image_end = . );
		_end_data = .;
	. = ALIGN(2);
	PROVIDE (_MYHEAP_START = . );
	PROVIDE (_MYHEAP_END = . + _MYHEAP_SIZE);
	} >RAM AT>FLASH
}

