/* This file defines standard ELF types, structures, and macros.
   Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ian Lance Taylor <ian@cygnus.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _ELF_H
#define	_ELF_H 1

#ifndef MINILIB
#include <stdint.h>
#endif

/* Standard ELF types.  */

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef	int32_t  Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef	int32_t  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef	int64_t  Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef	int64_t  Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;

/* Type of symbol indices.  */
typedef uint32_t Elf32_Symndx;
typedef uint64_t Elf64_Symndx;


/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct
{
  unsigned char	e_ident[EI_NIDENT];	/** Magic number and other info */
  Elf32_Half	e_type;			/** Object file type */
  Elf32_Half	e_machine;		/** Architecture */
  Elf32_Word	e_version;		/** Object file version */
  Elf32_Addr	e_entry;		/** Entry point virtual address */
  Elf32_Off	e_phoff;		/** Program header table file offset */
  Elf32_Off	e_shoff;		/** Section header table file offset */
  Elf32_Word	e_flags;		/** Processor-specific flags */
  Elf32_Half	e_ehsize;		/** ELF header size in bytes */
  Elf32_Half	e_phentsize;		/** Program header table entry size */
  Elf32_Half	e_phnum;		/** Program header table entry count */
  Elf32_Half	e_shentsize;		/** Section header table entry size */
  Elf32_Half	e_shnum;		/** Section header table entry count */
  Elf32_Half	e_shstrndx;		/** Section header string table index */
} Elf32_Ehdr;

/* Fields in the e_ident array.  The EI_* macros are indices into the
   array.  The macros under each EI_* macro are the values the byte
   may have.  */

#define EI_MAG0		0		/* File identification byte 0 index */
#define ELFMAG0		0x7f		/* Magic number byte 0 */

#define EI_MAG1		1		/* File identification byte 1 index */
#define ELFMAG1		'E'		/* Magic number byte 1 */

#define EI_MAG2		2		/* File identification byte 2 index */
#define ELFMAG2		'L'		/* Magic number byte 2 */

#define EI_MAG3		3		/* File identification byte 3 index */
#define ELFMAG3		'F'		/* Magic number byte 3 */

/* Conglomeration of the identification bytes, for easy testing as a word.  */
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define EI_CLASS	4		/* File class byte index */
#define ELFCLASSNONE	0		/* Invalid class */
#define ELFCLASS32	1		/* 32-bit objects */
#define ELFCLASS64	2		/* 64-bit objects */
#define ELFCLASSNUM	3

#define EI_DATA		5		/* Data encoding byte index */
#define ELFDATANONE	0		/* Invalid data encoding */
#define ELFDATA2LSB	1		/* 2's complement, little endian */
#define ELFDATA2MSB	2		/* 2's complement, big endian */
#define ELFDATANUM	3

#define EI_VERSION	6		/* File version byte index */
					/* Value must be EV_CURRENT */

#define EI_OSABI	7		/* OS ABI identification */
#define ELFOSABI_SYSV		0	/* UNIX System V ABI */
#define ELFOSABI_HPUX		1	/* HP-UX */
#define ELFOSABI_FREEBSD        9       /* Free BSD */
#define ELFOSABI_ARM		97	/* ARM */
#define ELFOSABI_STANDALONE	255	/* Standalone (embedded) application */

#define EI_ABIVERSION	8		/* ABI version */

#define EI_PAD		9		/* Byte index of padding bytes */

/* Legal values for e_type (object file type).  */

#define ET_NONE		0		/* No file type */
#define ET_REL		1		/* Relocatable file */
#define ET_EXEC		2		/* Executable file */
#define ET_DYN		3		/* Shared object file */
#define ET_CORE		4		/* Core file */
#define	ET_NUM		5		/* Number of defined types */
#define ET_LOPROC	0xff00		/* Processor-specific */
#define ET_HIPROC	0xffff		/* Processor-specific */

/* Legal values for e_machine (architecture).  */

#define EM_NONE		 0		/* No machine */
#define EM_M32		 1		/* AT&T WE 32100 */
#define EM_SPARC	 2		/* SUN SPARC */
#define EM_386		 3		/* Intel 80386 */
#define EM_68K		 4		/* Motorola m68k family */
#define EM_88K		 5		/* Motorola m88k family */
#define EM_486		 6		/* Intel 80486 */
#define EM_860		 7		/* Intel 80860 */
#define EM_MIPS		 8		/* MIPS R3000 big-endian */
#define EM_S370		 9		/* Amdahl */
#define EM_MIPS_RS4_BE	10		/* MIPS R4000 big-endian */
#define EM_RS6000	11		/* RS6000 */

#define EM_PARISC	15		/* HPPA */
#define EM_nCUBE	16		/* nCUBE */
#define EM_VPP500	17		/* Fujitsu VPP500 */
#define EM_SPARC32PLUS	18		/* Sun's "v8plus" */
#define EM_960		19		/* Intel 80960 */
#define EM_PPC		20		/* PowerPC */

#define EM_V800		36		/* NEC V800 series */
#define EM_FR20		37		/* Fujitsu FR20 */
#define EM_RH32		38		/* TRW RH32 */
#define EM_MMA		39		/* Fujitsu MMA */
#define EM_ARM		40		/* ARM */
#define EM_FAKE_ALPHA	41		/* Digital Alpha */
#define EM_SH		42		/* Hitachi SH */
#define EM_SPARCV9	43		/* SPARC v9 64-bit */
#define EM_TRICORE	44		/* Siemens Tricore */
#define EM_ARC		45		/* Argonaut RISC Core */
#define EM_H8_300	46		/* Hitachi H8/300 */
#define EM_H8_300H	47		/* Hitachi H8/300H */
#define EM_H8S		48		/* Hitachi H8S */
#define EM_H8_500	49		/* Hitachi H8/500 */
#define EM_IA_64	50		/* Intel Merced */
#define EM_MIPS_X	51		/* Stanford MIPS-X */
#define EM_COLDFIRE	52		/* Motorola Coldfire */
#define EM_68HC12	53		/* Motorola M68HC12 */
#define EM_NUM		54

/* If it is necessary to assign new unofficial EM_* values, please
   pick large random numbers (0x8523, 0xa7f2, etc.) to minimize the
   chances of collision with official or non-GNU unofficial values.  */

#define EM_ALPHA	0x9026

/* Legal values for e_version (version).  */

#define EV_NONE		0		/* Invalid ELF version */
#define EV_CURRENT	1		/* Current version */
#define EV_NUM		2

/* Section header.  */

typedef struct
{
  Elf32_Word	sh_name;		/** Section name (string tbl index) */
  Elf32_Word	sh_type;		/** Section type */
  Elf32_Word	sh_flags;		/** Section flags */
  Elf32_Addr	sh_addr;		/** Section virtual addr at execution */
  Elf32_Off	sh_offset;		/** Section file offset */
  Elf32_Word	sh_size;		/** Section size in bytes */
  Elf32_Word	sh_link;		/** Link to another section */
  Elf32_Word	sh_info;		/** Additional section information */
  Elf32_Word	sh_addralign;		/** Section alignment */
  Elf32_Word	sh_entsize;		/** Entry size if section holds table */
} Elf32_Shdr;

/* Special section indices.  */

#define SHN_UNDEF	0		/* Undefined section */
#define SHN_LORESERVE	0xff00		/* Start of reserved indices */
#define SHN_LOPROC	0xff00		/* Start of processor-specific */
#define SHN_HIPROC	0xff1f		/* End of processor-specific */
#define SHN_ABS		0xfff1		/* Associated symbol is absolute */
#define SHN_COMMON	0xfff2		/* Associated symbol is common */
#define SHN_HIRESERVE	0xffff		/* End of reserved indices */

/* Legal values for sh_type (section type).  */

#define SHT_NULL	 0		/* Section header table entry unused */
#define SHT_PROGBITS	 1		/* Program data */
#define SHT_SYMTAB	 2		/* Symbol table */
#define SHT_STRTAB	 3		/* String table */
#define SHT_RELA	 4		/* Relocation entries with addends */
#define SHT_HASH	 5		/* Symbol hash table */
#define SHT_DYNAMIC	 6		/* Dynamic linking information */
#define SHT_NOTE	 7		/* Notes */
#define SHT_NOBITS	 8		/* Program space with no data (bss) */
#define SHT_REL		 9		/* Relocation entries, no addends */
#define SHT_SHLIB	 10		/* Reserved */
#define SHT_DYNSYM	 11		/* Dynamic linker symbol table */
#define	SHT_NUM		 12		/* Number of defined types.  */
#define SHT_LOOS	 0x60000000	/* Start OS-specific */
#define SHT_LOSUNW	 0x6ffffffb	/* Sun-specific low bound.  */
#define SHT_SUNW_COMDAT  0x6ffffffb
#define SHT_SUNW_syminfo 0x6ffffffc
#define SHT_GNU_verdef	 0x6ffffffd	/* Version definition section.  */
#define SHT_GNU_verneed	 0x6ffffffe	/* Version needs section.  */
#define SHT_GNU_versym	 0x6fffffff	/* Version symbol table.  */
#define SHT_HISUNW	 0x6fffffff	/* Sun-specific high bound.  */
#define SHT_HIOS	 0x6fffffff	/* End OS-specific type */
#define SHT_LOPROC	 0x70000000	/* Start of processor-specific */
#define SHT_HIPROC	 0x7fffffff	/* End of processor-specific */
#define SHT_LOUSER	 0x80000000	/* Start of application-specific */
#define SHT_HIUSER	 0x8fffffff	/* End of application-specific */

/* Legal values for sh_flags (section flags).  */

#define SHF_WRITE	(1 << 0)	/* Writable */
#define SHF_ALLOC	(1 << 1)	/* Occupies memory during execution */
#define SHF_EXECINSTR	(1 << 2)	/* Executable */
#define SHF_MASKPROC	0xf0000000	/* Processor-specific */

/* --- Simplified ELF header --- */
typedef struct simple_elf {
  const char *  e_fname;       /* filename of binary */
  unsigned long e_entry;       /* entry point virtual address */
  unsigned long e_txtoff;      /* offset of text segment in file */
  unsigned long e_txtlen;      /* length of text segment in bytes */
  unsigned long e_txtstart;    /* start of text segment virtual address */
  unsigned long e_datoff;      /* offset of data segment in file */
  unsigned long e_datlen;      /* length of data segment in bytes */
  unsigned long e_datstart;    /* start of data segment in virtual memory */
  unsigned long e_rodatoff;    /* offset of rodata segment in file */
  unsigned long e_rodatlen;    /* length of rodata segment in bytes */
  unsigned long e_rodatstart;  /* start of rodata segment in virtual memory*/
  unsigned long e_bsslen;      /* length of bss  segment in bytes */
} simple_elf_t;

 /* --- Defines --- */
#define NMAGIC 0410 /* 0x108 */
#define N_MAGIC(exec) ((exec).a_midmag & 0xffff)
#define N_BADMAG(x) (N_MAGIC(x) != NMAGIC) 

#define ELF_NOTELF  -1
#define ELF_SUCCESS  0
#define NOT_PRESENT  -2

/*
 * elf function prototypes
 */
extern int getbytes( const char *filename, int offset, int size, char *buf);
extern int elf_load_helper( simple_elf_t *se_hdr, const char *fname );
extern int elf_check_header( const char *fname );

#endif	/* elf.h */
