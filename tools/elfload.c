#include <string.h>
#include <stdio.h>
#include "../include/elf.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define FATAL do { fprintf(stderr, "Error at %s:%d (%d) [%s]\n", \
  __FILE__, __LINE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE 131072
#define MAP_MASK (4096 - 1)

char * snarf_file(char * fname) {

	int fd = open(fname, O_RDONLY);
	if (fd < 0) FATAL;

	struct stat st;
	if (fstat(fd, &st) < 0) FATAL;

	char * buf = malloc(st.st_size);
	if (!buf) FATAL;
	if (read(fd, buf, st.st_size) != st.st_size) FATAL;

	printf("Loaded %d bytes.\n", st.st_size);
	return buf;
}

void dump_to_ram (void * src, int target, int size) {

	printf("Copying %d bytes to 0x%08x...\n", size, target);

	int fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (fd < 0) FATAL;

        void * map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, target & ~MAP_MASK);

        if (map_base == (void *) -1) FATAL;

        void * virt_addr = map_base + (target & MAP_MASK);
        memcpy(virt_addr, src, size);

}

static const unsigned char elf_ident[4] = { 0x7F, 'E', 'L', 'F' }; 

int load_elf (char * buf) {

	Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *) buf;
	Elf32_Shdr * elf_sec_hdrs = (Elf32_Shdr *) (buf + elf_hdr->e_shoff);

	if (memcmp(elf_hdr->e_ident, elf_ident, sizeof(elf_ident))) FATAL;
	if (elf_hdr->e_type != ET_EXEC) FATAL;
	if (elf_hdr->e_machine != EM_386) FATAL;
	if (elf_hdr->e_version != EV_CURRENT) FATAL;

	char * string_table = buf + elf_sec_hdrs[elf_hdr->e_shstrndx].sh_offset;

	int i;
	for (i = 0; i < elf_hdr->e_shnum; i++) {
		if (elf_sec_hdrs[i].sh_name == SHN_UNDEF) {
			continue;
		}

		char * section_name = string_table + elf_sec_hdrs[i].sh_name;

		if ((elf_sec_hdrs[i].sh_type != SHT_PROGBITS) || !(elf_sec_hdrs[i].sh_flags & SHF_ALLOC)) {
			printf("Skipping %s...\n", section_name);
			continue;
		}

		printf("Loading %s...\n", section_name);

		dump_to_ram(buf + elf_sec_hdrs[i].sh_offset,
		            elf_sec_hdrs[i].sh_addr,
		            elf_sec_hdrs[i].sh_size);
	}

	return 0;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s elffile\n", argv[0]);
		return 1;
	}

	printf("Loading file...\n");

	char * buf = snarf_file(argv[1]);
	load_elf(buf);

	return 0;
}

