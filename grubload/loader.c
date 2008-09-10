#include "minilib.h"
#include "../include/elf.h"

static const unsigned char elf_ident[4] = { 0x7F, 'E', 'L', 'F' }; 

int load_elf (char * buf, int size) {

	Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *) buf;
	Elf32_Shdr * elf_sec_hdrs = (Elf32_Shdr *) (buf + elf_hdr->e_shoff);

	/* Sanity check on ELF file */
	if (memcmp(elf_hdr->e_ident, elf_ident, sizeof(elf_ident))) return -1;
	if (elf_hdr->e_type != ET_EXEC) return -1;
	if (elf_hdr->e_machine != EM_386) return -1;
	if (elf_hdr->e_version != EV_CURRENT) return -1;
	if (size < sizeof(Elf32_Ehdr)) return -1;
	if (((char *)&elf_sec_hdrs[elf_hdr->e_shnum]) > (buf + size)) return -1;

	char * string_table = buf + elf_sec_hdrs[elf_hdr->e_shstrndx].sh_offset;

	if (string_table > (buf + size)) return -1;

	int i;
	for (i = 0; i < elf_hdr->e_shnum; i++) {

		if (elf_sec_hdrs[i].sh_name == SHN_UNDEF) {
			continue;
		}

		char * section_name = string_table + elf_sec_hdrs[i].sh_name;

		if ((elf_sec_hdrs[i].sh_type != SHT_PROGBITS) || !(elf_sec_hdrs[i].sh_flags & SHF_ALLOC)) {
			puts("Skipping ");
			puts(section_name);
			puts("\n");
			continue;
		}

		puts("Loading ");
		puts(section_name);
		puts("\n");

		memcpy(elf_sec_hdrs[i].sh_addr,
		       buf + elf_sec_hdrs[i].sh_offset,
		       elf_sec_hdrs[i].sh_size);
	}

	return 0;
}
