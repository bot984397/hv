#ifndef __LKM_MEM_H__
#define __LKM_MEM_H__

#include <linux/types.h>

u64 page_alloc (u8 order);

void page_free_safe (u64 addr, u8 order);
void page_free (u64 addr, u8 order);

void page_zero (void *addr, u8 order);
void page_set (void *addr, u8 val, u8 order);

u64 addr_virt_to_phys (void *addr);

#endif // __LKM_MEM_H__
