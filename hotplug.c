#define OLD_CPUHOTPLUG (LINUX_VERSION_CODE <= KERNEL_VERSION(4,8,0))

#include <linux/version.h>
#if OLD_CPUHOTPLUG
#include <linux/cpu.h>
#include <linux/notifier.h>
#else
#include <linux/cpuhotplug.h>
#endif
#include <asm/special_insns.h>
#include <asm/cpufeature.h>

#include "common.h"
#include "hotplug.h"
#include "cpu.h"

static int cpu_on_cb (unsigned int cpu)
{
   return 0;
}

static int cpu_off_cb (unsigned int cpu)
{
   return 0;
}

#if OLD_CPUHOTPLUG
static int cpu_hotplug_cb (struct notifier_block *nblock,
                           unsigned long action,
                           void *hcpu)
{
   switch (action)
   {
      case CPU_ONLINE:
         cpu_online_cb ((unsigned int)(unsigned long)hcpu);
         break;
      case CPU_OFFLINE:
         cpu_off_cb ((unsigned int)(unsigned long)hcpu);
         break;
   }
   return NOTIFY_OK;
}

struct notifier_block cpu_hotplug_notifier = {
   .notifier_call = cpu_hotplug_cb,
   .flags = NOTIFY_INIT_CALL_MASK
};
#endif // OLD_CPUHOTPLUG

__attribute__((warn_unused_result))
int cpu_hotplug_register (void)
{
#if OLD_CPUHOTPLUG
   register_cpu_notifier (&cpu_hotplug_notifier);
   return 1;
#else
   int ret = cpuhp_setup_state_nocalls (CPUHP_AP_ONLINE_DYN,
                                        "cpuhp:online",
                                        cpu_on_cb,
                                        cpu_off_cb);
   return ret <= 0 ? 0 : 1;
#endif // OLD_CPUHOTPLUG
}

void cpu_hotplug_unregister (void)
{
#if OLD_CPUHOTPLUG
   unregister_cpu_notifier (&cpu_hotplug_notifier);
#else
   cpuhp_remove_state_nocalls (CPUHP_AP_ONLINE_DYN);
#endif // OLD_CPUHOTPLUG
}
