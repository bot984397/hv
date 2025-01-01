#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "vmm.h"
#include "cpu.h"
#include "vmcs.h"
#include "common.h"
#include "hotplug.h"

vmm_ctx *g_vmm_ctx;

static int __init lkm_init (void)
{
   LOG_DBG ("module loaded");

   if (!cpu_hotplug_register ())
   {
      return -EPERM;
   }

   if (vmm_new () == false)
   {
      cpu_hotplug_unregister ();
      vmm_del ();
      return -ENOMEM;
   }

   on_each_cpu (cpu_init_pre, NULL, true);
   if (g_vmm_ctx->cpu_on != atomic_read (&g_vmm_ctx->cpu_init))
   {
      cpu_hotplug_unregister ();
      vmm_del ();
      return -EPERM;
   }

   // hypervisor should be up and running now.
   return 0;
}

static void __exit lkm_exit (void)
{
   LOG_DBG ("module unloaded");

   on_each_cpu (cpu_exit, NULL, true);
   cpu_hotplug_unregister ();
   vmm_del ();
}

module_init (lkm_init);
module_exit (lkm_exit);

MODULE_AUTHOR ("n1ghtc4ll");
MODULE_DESCRIPTION ("lightweight x86-64 vt-x hypervisor");
MODULE_LICENSE ("GPL");
