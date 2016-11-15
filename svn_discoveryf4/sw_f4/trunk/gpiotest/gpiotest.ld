OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)
/* Linker script for STM32F103RBT6, 128K flash, 20K RAM */

ENTRY (Reset_Handler);

/* Define memory regions. */
MEMORY
{
	RAM      (RWX) : ORIGIN = 0x20000000, LENGTH = 128K
	FLASH    (RX)  : ORIGIN = 0x08000000, LENGTH = 1024K
	CCM	 (RWX) : ORIGIN = 0x10000000, LENGTH = 64K
}

_MYHEAP_SIZE = 32K;

PROVIDE (_stack = 0x10000000 + 64K);
PROVIDE (_MSP   = 0x10000000);

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
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata.str1.*)
		. = ALIGN(4);
	} 	>FLASH

   /* .ARM.exidx is sorted, so has to go in its own output section.*/
   	.ARM.exidx : 
	{
		. = ALIGN(4);
        	__exidx_start = .;
        	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
        	__exidx_end = .;
 		 PROVIDE_HIDDEN (__exidx_end = .);
		__data_image = .;
		PROVIDE( __data_image = . );
    	} >FLASH



	.data	: ALIGN (4)
	{
		__data_section_start = .;
		PROVIDE ( __data_section_start = .); 
                *(.cs3.region-head.ram)
                KEEP(*(.jcr))
                *(.got.plt) *(.got)
                *(.shdata)
                *(.data .data.* .gnu.linkonce.d.*)
		__data_image_end = .;
	  	PROVIDE( __data_image_end = . );
	}  >RAM AT >FLASH

	.eh_frame_hdr : ALIGN (4)
	  {
	    KEEP (*(.eh_frame_hdr))
	  } >RAM AT >FLASH
  	.eh_frame : ALIGN (4)
  	{
    		KEEP (*(.eh_frame))
  	}  >RAM AT >FLASH

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
	PROVIDE (_MYHEAP_START = . );
	PROVIDE (_MYHEAP_END = . + _MYHEAP_SIZE);
		. = . + _MYHEAP_SIZE;
                 _ebss = .;        /* global needed for startup_stm3210x_md_vl.S */
    		_end_of_bss = .;
 	        _end = .;
                __end = .;
		PROVIDE (_end_of_bss = .);

  	}  >RAM 



}



