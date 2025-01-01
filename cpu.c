#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/slab.h>

#include "cpu.h"
#include "mem.h"
#include "vmcs.h"
#include "intrin.h"
#include "common.h"

_always_inline_ int cpu_this (void)
{
   return num_online_cpus ();
}

_always_inline_ int cpu_high (void)
{
   return smp_processor_id ();
}

static _warn_unused_result_ _always_inline_ int cpu_is_intel (void)
{
   u32 r[4] = {0};
   cpuid1 (0, &r[0], &r[1], &r[2], &r[3]);

   // some 2015 haswell processors have the faulty CPUID "GenuineIotel"
   return (r[1] == 0x756E6547 && 
          (r[2] == 0x6C65746E || r[2] == 0x6C65746F) &&
           r[3] == 0x49656E69);
}

static _warn_unused_result_ _always_inline_ int cpu_probe_vmx (void)
{
   if (!cpu_is_intel ())
   {
      return 0;
   }

   // if bit 0 is clear, UEFI or BIOS firmware fucked up
   ia32_feature_control fctl = {0};
   fctl.ctl = rdmsr1 (IA32_FEATURE_CONTROL);
   return (fctl.locked == 0 || (fctl.locked == 1 && fctl.vos == 0));
}

static void cpu_enable_vmx (void)
{
   u64 cr0 = readcr0 ();
   cr0 |= rdmsr1 (IA32_VMX_CR0_FIXED0) & 0xFFFFFFFF;
   cr0 &= rdmsr1 (IA32_VMX_CR0_FIXED1) & 0xFFFFFFFF;
   writecr0 (cr0);

   u64 cr4 = readcr4 ();
   cr4 |= (1ULL << 13);
   cr4 |= rdmsr1 (IA32_VMX_CR4_FIXED0) & 0xFFFFFFFF;
   cr4 &= rdmsr1 (IA32_VMX_CR4_FIXED1) & 0xFFFFFFFF;
   writecr4 (cr4);
}

static int cpu_new_vmx_region (vmx_reg **region, u64 *phys)
{
   *region = (vmx_reg *)page_alloc (0);
   if (!*region)
   {
      return 0;
   }
   *phys = addr_virt_to_phys (*region);
   
   ia32_vmx_basic vb = { .ctl = rdmsr1 (IA32_VMX_BASIC) };
   (*region)->header.rev_ident = vb.vmcs_rev_ident;
   (*region)->header.reserved_0 = 0;

   return 1;
}

static int cpu_new_bitmap (u8 **bitmap, u64 *phys)
{
   *bitmap = (u8 *)page_alloc (0);
   if (!*bitmap)
   {
      return 0;
   }
   *phys = addr_virt_to_phys (*bitmap);
   return 1;
}

_warn_unused_result_ cpu_ctx* cpu_new (void)
{
   u64 e = 0;

   cpu_ctx *_cpu_ctx = _kmalloc (sizeof (cpu_ctx));
   if (!_cpu_ctx) { return NULL; }

   e |= cpu_new_vmx_region (&_cpu_ctx->vmxon_region, 
                            &_cpu_ctx->vmxon_physical);
   e |= cpu_new_vmx_region (&_cpu_ctx->vmcs_region,
                            &_cpu_ctx->vmcs_physical);

   e |= cpu_new_bitmap (&_cpu_ctx->bitmaps.io_bitmap_a,
                        &_cpu_ctx->bitmaps.io_bitmap_a_phys);
   e |= cpu_new_bitmap (&_cpu_ctx->bitmaps.io_bitmap_b,
                        &_cpu_ctx->bitmaps.io_bitmap_b_phys);
   e |= cpu_new_bitmap (&_cpu_ctx->bitmaps.msr_bitmaps,
                        &_cpu_ctx->bitmaps.msr_bitmaps_phys);

   if (e)
   {
      cpu_del (_cpu_ctx);
      return NULL;
   } return _cpu_ctx;
}

void cpu_del (cpu_ctx *_cpu_ctx)
{
   if (_cpu_ctx)
   {
      page_free_safe ((unsigned long)_cpu_ctx->vmxon_region, 0);
      page_free_safe ((unsigned long)_cpu_ctx->vmcs_region, 0);
      page_free_safe ((unsigned long)_cpu_ctx->bitmaps.io_bitmap_a, 0);
      page_free_safe ((unsigned long)_cpu_ctx->bitmaps.io_bitmap_b, 0);
      page_free_safe ((unsigned long)_cpu_ctx->bitmaps.msr_bitmaps, 0);

      kfree (_cpu_ctx);
   }
}

void cpu_init_pre (_unused_ void *info)
{
   if (!cpu_probe_vmx ())
   {
      return;
   }

   cpu_enable_vmx ();
   
   cpu_ctx *_cpu_ctx = g_vmm_ctx->cpu_ctx[cpu_this ()];
   if (vmxon (_cpu_ctx->vmxon_physical))
   {
      vmxoff ();
      return;
   }

   int rax = cpu_init_asm (_cpu_ctx);
   // todo: return error in rax or rflags
   if (rax)
   {
      _cpu_ctx->launched = 1;
      atomic_inc (&g_vmm_ctx->cpu_init);
   }
}

int cpu_init_main (cpu_ctx *_cpu_ctx, u64 g_sp, u64 g_ip)
{
   return 0;
}

void cpu_exit (_unused_ void *info)
{
   if (!vmxoff ())
   {
      cpu_ctx *_cpu_ctx = g_vmm_ctx->cpu_ctx[cpu_this ()];
      _cpu_ctx->launched = 0;
   }
}
