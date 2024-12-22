#include "mem.h"
#include "common.h"

#include <linux/gfp.h>
#include <asm/io.h>

unsigned long mem_alloc_pages (unsigned int order)
{
   unsigned long page_addr = __get_free_pages (GFP_KERNEL, order);
   if (!page_addr)
   {
      LOG_DBG ("failed to allocate pages - order: %d\n", order);
      return -ENOMEM;
   }
   if (page_addr & (PAGE_SIZE - 1))
   {
      LOG_DBG ("allocated pages were not page aligned.\n");
      mem_free_pages (page_addr, order);
      return -ENOMEM;
   }
   return page_addr;
}

void mem_free_pages_s (unsigned long page_addr, unsigned int order)
{
   if (!page_addr)
   {
      return;
   }
   mem_free_pages (page_addr, order);
}

void mem_free_pages (unsigned long page_addr, unsigned int order)
{
   free_pages (page_addr, order);
}

void mem_zero_pages (void *page, unsigned int order)
{
   if (!page)
   {
      return;
   }
   for (int i = 0; i < (1 << order); i++)
   {
      clear_page (page + (i * PAGE_SIZE));
   }
}

void mem_set_pages (void *page, u8 val, unsigned int order)
{
   if (!page)
   {
      return;
   }
   memset (page, val, PAGE_SIZE << order);
}

u64 mem_virt_to_phys (void *virt_addr)
{
   return (u64)virt_to_phys (virt_addr);
}
