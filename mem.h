// mem.h

#ifndef __LKM_MEM_H__
#define __LKM_MEM_H__

#include <linux/types.h>

unsigned long mem_alloc_pages (unsigned int order);
void mem_free_pages (unsigned long page_addr, unsigned int order);
u64 mem_virt_to_phys (void *virt_addr);

#endif // __LKM_MEM_H__

// eof
