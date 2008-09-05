OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
SECTIONS
{
	. = 0xA8000;
	
	.aseg 0xA8000 : {
		_start = .;
		aseg.o
	}
}
