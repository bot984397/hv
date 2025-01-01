#include <linux/slab.h>
#include <linux/mm.h>

#include "cpu.h"
#include "mem.h"
#include "common.h"

bool vmm_alloc (void)
{
   g_vmm_ctx = kmalloc (sizeof (vmm_ctx), GFP_KERNEL);
   if (g_vmm_ctx == NULL)
   {
      return false;
   }

   g_vmm_ctx->vcpu_max = max_logical_cpu ();
   atomic_set (&g_vmm_ctx->vcpu_init, 0);

   g_vmm_ctx->vcpu_ctxs 
      = kmalloc (sizeof (cpu_ctx *) * g_vmm_ctx->vcpu_max, GFP_KERNEL);
   if (g_vmm_ctx->vcpu_ctxs == NULL)
   {
      return false;
   }

   for (int i = 0; i < g_vmm_ctx->vcpu_max; i++)
   {
      g_vmm_ctx->vcpu_ctxs[i] = vcpu_alloc ();
      if (g_vmm_ctx->vcpu_ctxs[i] == NULL)
      {
         return false;
      }
   }

   return true;
}

void vmm_free (void)
{
   if (!g_vmm_ctx)
   {
      return;
   }

   for (int i = 0; i < g_vmm_ctx->vcpu_max; i++)
   {
      if (g_vmm_ctx->vcpu_ctxs[i])
      {
         vcpu_free (g_vmm_ctx->vcpu_ctxs[i]);
      }
   }
   kfree (g_vmm_ctx);
}
