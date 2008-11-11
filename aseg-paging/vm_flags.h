/** Page table and page directory flags.
 *
 *  From intel-sys.pdf.
 *
 *  @author Jacob Potter (jdpotter)
 */

#ifndef _VM_FLAGS_H
#define _VM_FLAGS_H

#define PTE_FOR(x)      (((unsigned int)(x) >> 12) & 0x3FF)
#define PDE_FOR(x)      ((unsigned int)(x) >> 22)
#define ADDR_12_MASK(x)	((unsigned int)(x) & ~((1 << 12) - 1))
#define ADDR_22_MASK(x)	((unsigned int)(x) & ~((1 << 22) - 1))
#define LOWER_12(x)	((unsigned int)(x) & ((1 << 12) - 1))
#define LOWER_22(x)	((unsigned int)(x) & ((1 << 22) - 1))

#define PDE_4M_ADDR_SHIFT       22
#define PTE_4K_ADDR_SHIFT       12
#define PDE_TABLE_ADDR_SHIFT    12
#define PTE_FRAME_ADDR_MASK     (~((1 << PTE_4K_ADDR_SHIFT) - 1))
#define PDE_TABLE_ADDR_MASK     (~((1 << PDE_TABLE_ADDR_SHIFT) - 1))

#define PDE_ATTRIB_INDEX	(1 << 12)
#define PDE_GLOBAL		(1 << 8)
#define PDE_PAGE_SIZE		(1 << 7)
#define PDE_DIRTY		(1 << 6)
#define PDE_ACCESSED		(1 << 5)
#define PDE_NO_CACHE		(1 << 4)
#define PDE_WRITE_THROUGH	(1 << 3)
#define PDE_USER		(1 << 2)
#define PDE_READ_WRITE		(1 << 1)
#define PDE_PRESENT		(1 << 0)

#define PTE_GLOBAL		(1 << 8)
#define PTE_ATTRIB_INDEX	(1 << 7)
#define PTE_DIRTY		(1 << 6)
#define PTE_ACCESSED		(1 << 5)
#define PTE_NO_CACHE		(1 << 4)
#define PTE_WRITE_THROUGH	(1 << 3)
#define PTE_USER		(1 << 2)
#define PTE_READ_WRITE		(1 << 1)
#define PTE_PRESENT		(1 << 0)

#endif /* _VM_FLAGS_H */
