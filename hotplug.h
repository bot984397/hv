#ifndef __LKM_HOTPLUG_H__
#define __LKM_HOTPLUG_H__

__attribute__((warn_unused_result))
int cpu_hotplug_register (void);

void cpu_hotplug_unregister (void);

#endif // __LKM_HOTPLUG_H__
