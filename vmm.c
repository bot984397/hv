#include <linux/slab.h>
#include <linux/mm.h>

#include "cpu.h"
#include "mem.h"
#include "common.h"

bool vmm_alloc (void)
{
   g_vmm_ctx = kmalloc (sizeof (vmm_ctx_t), GFP_KERNEL);
   if (g_vmm_ctx == NULL)
   {
      LOG_DBG ("failed to allocate vmm_ctx\n");
      return false;
   }

   g_vmm_ctx->vcpu_max = max_logical_cpu ();
   atomic_set (&g_vmm_ctx->vcpu_init, 0);

   g_vmm_ctx->vcpu_ctxs 
      = kmalloc (sizeof (vcpu_ctx_t *) * g_vmm_ctx->vcpu_max, GFP_KERNEL);
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

      /*
      vcpu_ctx_t *vcpu_ctx = vcpu_alloc ();
      if (vcpu_ctx == NULL)
      {
         return false;
      }
      vcpu_ctx->cpu_num = i;

      vcpu_ctx_t *list_head = g_vmm_ctx->vcpu_ctx_ll;
      if (!list_head)
      {
         list_head = vcpu_ctx;
      }
      else
      {
         while (list_head->flink)
         {
            list_head = list_head->flink;
         }
         list_head->flink = vcpu_ctx;
         vcpu_ctx->blink = list_head;
      }
      */
   }
   return true;
}

void vmm_free (vmm_ctx_t *vmm_ctx)
{
   if (!vmm_ctx)
   {
      return;
   }

   for (int i = 0; i < vmm_ctx->vcpu_max; i++)
   {
      if (vmm_ctx->vcpu_ctxs[i])
      {
         vcpu_free (vmm_ctx->vcpu_ctxs[i]);
      }
   }
   kfree (vmm_ctx);
}
