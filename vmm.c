#include <linux/slab.h>
#include <linux/mm.h>

#include "cpu.h"
#include "mem.h"
#include "common.h"

int vmm_new (void)
{
   g_vmm_ctx = _kmalloc (sizeof (vmm_ctx));
   if (!g_vmm_ctx) { return 0; }

   g_vmm_ctx->cpu_on = cpu_high ();
   atomic_set (&g_vmm_ctx->cpu_init, 0);

   g_vmm_ctx->cpu_ctx = _kmalloc (sizeof (cpu_ctx *) * g_vmm_ctx->cpu_on);
   if (!g_vmm_ctx->cpu_ctx) { return 0; }

   for (u16 i = 0; i < g_vmm_ctx->cpu_on; i++)
   {
      g_vmm_ctx->cpu_ctx[i] = cpu_new ();
      if (!g_vmm_ctx->cpu_ctx[i]) { return 0; }
   }

   return 1;
}

void vmm_del (void)
{
   if (!g_vmm_ctx)
   {
      return;
   }

   for (u16 i = 0; i < g_vmm_ctx->cpu_on; i++)
   {
      if (g_vmm_ctx->cpu_ctx[i])
      {
         cpu_del (g_vmm_ctx->cpu_ctx[i]);
      }
   }
   kfree (g_vmm_ctx);
}
