#ifndef __LKM_CPU_H__
#define __LKM_CPU_H__

#include "common.h"
#include "vmm.h"

#define VCPU_DBG(fmt, ...) \
   LOG_DBG ("cpu %02d - " fmt, cur_logical_cpu (), ##__VA_ARGS__)

int max_logical_cpu (void);
int cur_logical_cpu (void);

int cpu_this (void);
int cpu_high (void);

int cpu_probe_vmx (void);

cpu_ctx* cpu_new (void);
void cpu_del (cpu_ctx *_cpu_ctx);

int cpu_init_pre (void);
extern int cpu_init_asm (cpu_ctx *_cpu_ctx);
int cpu_init_main (cpu_ctx *_cpu_ctx, u64 g_sp, u64 g_ip);

void vcpu_init (void *info);
void vcpu_restore (void *info);

int cpu_init_single (void);

#endif // __LKM_CPU_H__
