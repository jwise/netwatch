OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS
{
	. = 0xA8000;
	
	.aseg : {
		_start = .;
		entry.o
		LONG(0xA9FF0);
		LONG(c_entry);
		entry_initialized = .;
		LONG(0);
		pagingstub-asm.o
		pagingstub.o
		_aseg_end = .;
	}

	. = 0x200000;

	.text : {
		*(.text);
		*(SORT(.table.*));
	}
	.data : { *(.data); *(.data2); }
	.rodata : { *(.rodata); }
	.text : { *(.text); }
	
	_bss = .;
	.bss : { *(.bss); }
	_bssend = .;
	_end = .;

	.stack : { 
		. = . + 0x10000;
		. = ALIGN(0x10);
		_primary_stack_top = .;
	}

	. = 0x10000;
	.info : {
		_start = .;
		LONG(0x5754454E);
		LONG(__firstrun_stub);
	}

}
