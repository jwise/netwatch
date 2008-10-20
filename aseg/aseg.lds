OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS
{
	. = 0xA8000;
	.aseg : {
		_start = .;
		aseg.o
		LONG(_stacktop);
		LONG(smi_entry);
		*(.data);
		*(.rodata);

		. = 0x8000;
		*(.text);
	}

	. = 0x1FF80000;
	.bss : {
		_bss = .;
		*(.bss);
		_bssend = .;
	}
	.stack : { 
		. = . + 0x10000;
		. = ALIGN(0x10);
		_stacktop = .;
	}

	. = 0x10000;
	.info : {
		_start = .;
		LONG(0x5754454E);
		LONG(__firstrun_start);
	}
}
