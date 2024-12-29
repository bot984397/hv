#ifndef __LKM_CPU_H__
#define __LKM_CPU_H__

#include "common.h"
#include "vmm.h"

#define VCPU_DBG(fmt, ...) \
   LOG_DBG ("cpu %02d - " fmt, cur_logical_cpu (), ##__VA_ARGS__)

vcpu_ctx_t* vcpu_ctx_from_cpu_num (u32 cpu_num);

int max_logical_cpu (void);
int cur_logical_cpu (void);

vcpu_ctx_t* vcpu_alloc (void);
void vcpu_free (vcpu_ctx_t *vcpu_ctx);

void vcpu_init (void *info);
void vcpu_restore (void *info);

#endif // __LKM_CPU_H__
