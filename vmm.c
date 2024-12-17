#include <linux/slab.h>
#include <linux/mm.h>

#include "cpu.h"
#include "mem.h"
#include "common.h"

vmm_ctx_t* vmm_alloc (void)
{
   vmm_ctx_t *vmm_ctx = kmalloc (sizeof (vmm_ctx_t), GFP_KERNEL);
   if (vmm_ctx == NULL)
   {
      LOG_DBG ("failed to allocate vmm_ctx\n");
      return NULL;
   }

   vmm_ctx->vcpu_max = max_logical_cpu ();
   atomic_set (&vmm_ctx->vcpu_init, 0);

   vmm_ctx->vcpu_ctxs = 
      kmalloc (sizeof (vcpu_ctx_t *) * vmm_ctx->vcpu_max, GFP_KERNEL);
   if (vmm_ctx->vcpu_ctxs == NULL)
   {
      LOG_DBG ("failed to allocate vcpu_ctxs\n");
      kfree (vmm_ctx);
      return NULL;
   }

   for (int i = 0; i < vmm_ctx->vcpu_max; i++)
   {
      vmm_ctx->vcpu_ctxs[i] = vcpu_alloc ();
      if (vmm_ctx->vcpu_ctxs[i] == NULL)
      {
         return NULL;
      }
   }
   
   return vmm_ctx;
}

vmm_ctx_t* vmm_init (void)
{
   vmm_ctx_t *vmm_ctx = kmalloc (sizeof (vmm_ctx_t), GFP_KERNEL);
   if (vmm_ctx == NULL)
   {
      LOG_DBG ("failed to allocate vmm_ctx\n");
      return NULL;
   }

   vmm_ctx->vcpu_max = max_logical_cpu ();
   atomic_set (&vmm_ctx->vcpu_init, 0);

   vmm_ctx->vcpu_ctxs = kmalloc (sizeof (vcpu_ctx_t *) * vmm_ctx->vcpu_max, 
                                 GFP_KERNEL);
   if (vmm_ctx->vcpu_ctxs == NULL)
   {
      LOG_DBG ("failed to allocate vcpu context array\n");
      kfree (vmm_ctx);
      return NULL;
   }

   for (int i = 0; i < vmm_ctx->vcpu_max; i++)
   {
      vmm_ctx->vcpu_ctxs[i] = vcpu_alloc ();
      if (vmm_ctx->vcpu_ctxs[i] == NULL)
      {
         return NULL;
      }
   }

   return vmm_ctx;
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
