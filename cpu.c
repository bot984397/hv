#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/slab.h>

#include "cpu.h"
#include "mem.h"
#include "vmcs.h"
#include "intrin.h"
#include "common.h"

_always_inline_ int cpu_high (void)
{
   return num_online_cpus ();
}

_always_inline_ int cpu_this (void)
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
   return (fctl.locked == 1 && fctl.vos == 1);
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
      return 1;
   }
   page_zero (*region, 0);
   *phys = addr_virt_to_phys (*region);
   
   ia32_vmx_basic vb = { .ctl = rdmsr1 (IA32_VMX_BASIC) };
   (*region)->header.rev_ident = vb.vmcs_rev_ident;
   (*region)->header.reserved_0 = 0;

   return 0;
}

static int cpu_new_bitmap (u8 **bitmap, u64 *phys)
{
   *bitmap = (u8 *)page_alloc (0);
   if (!*bitmap)
   {
      return 1;
   }
   *phys = addr_virt_to_phys (*bitmap);
   return 0;
}

static int cpu_new_cache (cpu_ctx *_cpu_ctx)
{
   _cpu_ctx->cached.vmx_basic.ctl = rdmsr1 (IA32_VMX_BASIC);

   return 0;
}

static int cpu_new_stack (cpu_ctx *_cpu_ctx)
{
#if KMALLOC_ALIGNED
   _cpu_ctx->_vmm_stk = (vmm_stk *)_kmalloc (sizeof (vmm_stk));
#else
   _cpu_ctx->_vmm_stk = (vmm_stk *)page_alloc (3);
#endif
   if (!_cpu_ctx->_vmm_stk) return 1;
   _cpu_ctx->_vmm_stk->_cpu_ctx = _cpu_ctx;
   return 0;
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

   e |= cpu_new_stack (_cpu_ctx);

   e |= cpu_new_cache (_cpu_ctx);

   if (e)
   {
      cpu_del (_cpu_ctx);
      return NULL;
   } 
   return _cpu_ctx;
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

#if KMALLOC_ALIGNED
      kfree (_cpu_ctx->_vmm_stk);
#else
      page_free_safe ((unsigned long)_cpu_ctx->_vmm_stk, 3);
#endif

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
   
   cpu_ctx *_cpu_ctx = g_vmm_ctx->_cpu_ctx[cpu_this ()];
   if (vmxon (_cpu_ctx->vmxon_physical) != 0)
   {
      return;
   }

   if (!cpu_init_asm (_cpu_ctx))
   {
      vmxoff ();
      return;
   }

   _cpu_ctx->launched = 1;
   atomic_inc (&g_vmm_ctx->cpu_init);
}

int cpu_init_main (cpu_ctx *_cpu_ctx, u64 g_sp, u64 g_rf, u64 g_ip)
{
   int e = vmcs_init_all (_cpu_ctx, (guest_init){ g_sp, g_rf, g_ip });
   if (e != 0)
   {
      LOG_DBG ("error: %d", e);
      return 1;
   }

   return 0;
}

void cpu_exit (_unused_ void *info)
{
   if (!vmxoff ())
   {
      cpu_ctx *_cpu_ctx = g_vmm_ctx->_cpu_ctx[cpu_this ()];
      _cpu_ctx->launched = 0;
   }
}
