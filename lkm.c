#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "vmm.h"
#include "cpu.h"
#include "msr.h"
#include "vmcs.h"
#include "common.h"
#include "hotplug.h"

vmm_ctx_t *g_vmm_ctx;

static int __init lkm_init (void)
{
   LOG_DBG ("module loaded");

   if (!cpu_hotplug_register ())
   {
      return -EPERM;
   }

   if (!vmm_alloc ())
   {
      cpu_hotplug_unregister ();
      vmm_free (g_vmm_ctx);
      return -ENOMEM;
   }

   on_each_cpu (vcpu_init, (void *)g_vmm_ctx, true);
   if (g_vmm_ctx->vcpu_max != atomic_read (&g_vmm_ctx->vcpu_init))
   {
      LOG_DBG ("one or more processors failed to enter vmx operation");
      cpu_hotplug_unregister ();
      vmm_free (g_vmm_ctx);
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
   vmm_free (g_vmm_ctx);
   LOG_DBG ("hypervisor unloaded");
}

module_init (lkm_init);
module_exit (lkm_exit);

MODULE_AUTHOR ("n1ghtc4ll");
MODULE_DESCRIPTION ("lightweight x86-64 vt-x hypervisor");
MODULE_LICENSE ("GPL");
