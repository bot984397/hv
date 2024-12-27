#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "vmm.h"
#include "cpu.h"
#include "msr.h"
#include "asm.h"
#include "vmcs.h"
#include "common.h"
#include "hotplug.h"

vmm_ctx_t *g_vmm_ctx;

static void debug (void)
{
   vcpu_ctx_t *vcpu_ctx;
   vcpu_ctx = vcpu_alloc ();
   if (vcpu_ctx == NULL)
   {
      LOG_DBG ("failure");
   }
   else
   {
      LOG_DBG ("success");
      vcpu_free (vcpu_ctx);
   }
}

static int __init lkm_init (void)
{
   LOG_DBG ("module loaded");

   if (!cpu_hotplug_register ())
   {
      return -EPERM;
   }

   if (vmm_alloc () == false)
   {
      cpu_hotplug_unregister ();
      vmm_free ();
      return -ENOMEM;
   }

   on_each_cpu (vcpu_init, (void *)g_vmm_ctx, true);
   if (g_vmm_ctx->vcpu_max != atomic_read (&g_vmm_ctx->vcpu_init))
   {
      LOG_DBG ("one or more processors failed to enter vmx operation");
      cpu_hotplug_unregister ();
      vmm_free ();
      return -EPERM;
   }

   // hypervisor should be up and running now.
   LOG_DBG ("hypervisor loaded");
   return 0;
}

static void __exit lkm_exit (void)
{
   LOG_DBG ("module unloaded");

   on_each_cpu (vcpu_restore, NULL, true);

   cpu_hotplug_unregister ();
   vmm_free ();
   LOG_DBG ("hypervisor unloaded");
}

module_init (lkm_init);
module_exit (lkm_exit);

MODULE_AUTHOR ("n1ghtc4ll");
MODULE_DESCRIPTION ("lightweight x86-64 vt-x hypervisor");
MODULE_LICENSE ("GPL");
