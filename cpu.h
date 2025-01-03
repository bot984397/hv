#ifndef __LKM_CPU_H__
#define __LKM_CPU_H__

#include "common.h"
#include "vmm.h"

#define VCPU_DBG(fmt, ...) \
   LOG_DBG ("cpu %02d - " fmt, cpu_this (), ##__VA_ARGS__)

int cpu_this (void);
int cpu_high (void);

cpu_ctx* cpu_new (void);
void cpu_del (cpu_ctx *_cpu_ctx);

void cpu_init_pre (_unused_ void *info);
extern int cpu_init_asm (cpu_ctx *_cpu_ctx);
int cpu_init_main (cpu_ctx *_cpu_ctx, u64 g_sp, u64 g_rf, u64 g_ip);

void cpu_exit (_unused_ void *info);

#endif // __LKM_CPU_H__
