#include "mem.h"
#include "common.h"

#include <linux/gfp.h>
#include <asm/io.h>

void* _kmalloc (u64 size)
{
   return kmalloc (size, GFP_KERNEL);
}

u64 page_alloc (u8 order)
{
   u64 addr = __get_free_pages (GFP_KERNEL, order);
   if (!addr)
   {
      LOG_DBG ("failed to allocate pages - order: %d\n", order);
      return -ENOMEM;
   }
   if (addr & (PAGE_SIZE - 1))
   {
      LOG_DBG ("allocated pages were not page aligned.\n");
      page_free (addr, order);
      return -ENOMEM;
   }
   return addr;
}

void page_free_safe (u64 addr, u8 order)
{
   if (!addr)
   {
      return;
   }
   page_free (addr, order);
}

void page_free (u64 addr, u8 order)
{
   free_pages (addr, order);
}

void page_zero (void *addr, u8 order)
{
   if (!addr)
   {
      return;
   }
   for (int i = 0; i < (1 << order); i++)
   {
      clear_page (addr + (i * PAGE_SIZE));
   }
}

void page_set (void *addr, u8 val, u8 order)
{
   if (addr)
   {
      memset (addr, val, PAGE_SIZE << order);
   }
}

u64 addr_virt_to_phys (void *addr)
{
   return (u64)virt_to_phys (addr);
}
