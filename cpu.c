#define VCPU_DBG(fmt, ...) \
   LOG_DBG ("cpu %02d - " fmt, cur_logical_cpu (), ##__VA_ARGS__)

#define KMALLOC_ALIGNED (LINUX_VERSION_CODE >= KERNEL_VERSION (5,4,0))

#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/special_insns.h>
#include <asm/cpufeature.h>

#include "cpu.h"
#include "crx.h"
#include "msr.h"
#include "mem.h"
#include "enc.h"
#include "vmcs.h"
#include "intrin.h"
#include "common.h"

int max_logical_cpu (void)
{
   return num_online_cpus ();
}

int cur_logical_cpu (void)
{
   return smp_processor_id ();
}

static inline __attribute__((always_inline))
bool vcpu_supports_vmx (void)
{
   return this_cpu_has (X86_FEATURE_VMX);
}

static __attribute__((warn_unused_result))
bool vcpu_enable_vmx (void)
{
   cr0_t cr0 = {0};
   cr4_t cr4 = {0};
   ia32_feature_control_t feat_ctl = {0};
   ia32_gen_fixed_t fixed = {0};

   
   feat_ctl.ctl = __rdmsr (IA32_FEATURE_CONTROL_MSR);
   if (feat_ctl.locked == 1 && feat_ctl.vmx_outside_smx == 0)
   {
      return false;
   }

   if (feat_ctl.locked == 0)
   {
      feat_ctl.locked = 1;
      feat_ctl.vmx_outside_smx = 1;
      __wrmsr (IA32_FEATURE_CONTROL_MSR, (feat_ctl.ctl >> 32), feat_ctl.ctl);
   }

   cr4.ctl = __read_cr4 ();
   cr4.VMXE = 1;
   fixed.ctl = __rdmsr (IA32_VMX_CR4_FIXED0_MSR);
   cr4.ctl |= fixed.split.low;
   fixed.ctl = __rdmsr (IA32_VMX_CR4_FIXED1_MSR);
   cr4.ctl &= fixed.split.low;
   __write_cr4 (cr4.ctl);

   cr0.value = __read_cr0 ();
   fixed.ctl = __rdmsr (IA32_VMX_CR0_FIXED0_MSR);
   cr0.value |= fixed.split.low;
   fixed.ctl = __rdmsr (IA32_VMX_CR0_FIXED1_MSR);
   cr0.value &= fixed.split.low;
   __write_cr0 (cr0.value);

   return true;
}

static __attribute__((warn_unused_result)) 
bool vcpu_alloc_vmx_region (vm_region_t **region, u64 *phys)
{
   ia32_vmx_basic_t vmx_basic = {0};
   vmx_basic.ctl = __rdmsr (IA32_VMX_BASIC_MSR);
#ifdef KMALLOC_ALIGNEDS
   *region = kzalloc (vmx_basic.vmx_region_size, GFP_KERNEL);
   if (*region == NULL)
   {
      return false;
   }
#else
   *region = (vm_region_t *)mem_alloc_pages (0);
   if (*region == NULL)
   {
      return false;
   }
#endif // KMALLOC_ALIGNED
   *phys = mem_virt_to_phys (*region);
   
   (*region)->header.rev_ident = vmx_basic.vmcs_rev_ident;
   (*region)->header.reserved_0 = 0;

   return true;
}

__attribute__((warn_unused_result)) vcpu_ctx_t* vcpu_alloc (void)
{
   vcpu_ctx_t *vcpu_ctx;

   vcpu_ctx = kmalloc (sizeof (vcpu_ctx_t), GFP_KERNEL);
   if (vcpu_ctx == NULL)
   {
      goto FAILURE;
   }

   if (!vcpu_alloc_vmx_region (&vcpu_ctx->vmxon_region, 
                               &vcpu_ctx->vmxon_physical))
   {
      goto FAILURE;
   }

   if (!vcpu_alloc_vmx_region (&vcpu_ctx->vmcs_region,
                               &vcpu_ctx->vmcs_physical))
   {
      goto FAILURE;
   }

   vcpu_ctx->bitmaps.io_bitmap_a = (u8 *)mem_alloc_pages (0);
   if (vcpu_ctx->bitmaps.io_bitmap_a == NULL)
   {
      goto FAILURE;
   }
   vcpu_ctx->bitmaps.io_bitmap_a_phys
      = mem_virt_to_phys (vcpu_ctx->bitmaps.io_bitmap_a);

   vcpu_ctx->bitmaps.io_bitmap_b = (u8 *)mem_alloc_pages (0);
   if (vcpu_ctx->bitmaps.io_bitmap_b == NULL)
   {
      goto FAILURE;
   }
   vcpu_ctx->bitmaps.io_bitmap_b_phys
      = mem_virt_to_phys (vcpu_ctx->bitmaps.io_bitmap_b);

   vcpu_ctx->bitmaps.msr_bitmaps = (u8 *)mem_alloc_pages (0);
   if (vcpu_ctx->bitmaps.msr_bitmaps == NULL)
   {
      goto FAILURE;
   }
   vcpu_ctx->bitmaps.msr_bitmaps_phys
      = mem_virt_to_phys (vcpu_ctx->bitmaps.msr_bitmaps);

   vcpu_ctx->cached.vmx_basic.ctl = __rdmsr (IA32_VMX_BASIC_MSR);

   return vcpu_ctx;

FAILURE:
   vcpu_free (vcpu_ctx);
   return NULL;
}

void vcpu_free (vcpu_ctx_t *vcpu_ctx)
{
   if (!vcpu_ctx)
   {
      return;
   }

#ifdef KMALLOC_ALIGNEDS
   if (vcpu_ctx->vmxon_region) kfree (vcpu_ctx->vmxon_region);
   if (vcpu_ctx->vmcs_region) kfree (vcpu_ctx->vmcs_region);
#else
   mem_free_pages_s ((unsigned long)vcpu_ctx->vmxon_region, 0);
   mem_free_pages_s ((unsigned long)vcpu_ctx->vmcs_region, 0);
#endif
   mem_free_pages_s ((unsigned long)vcpu_ctx->bitmaps.io_bitmap_a, 0);
   mem_free_pages_s ((unsigned long)vcpu_ctx->bitmaps.io_bitmap_b, 0);
   mem_free_pages_s ((unsigned long)vcpu_ctx->bitmaps.msr_bitmaps, 0);

   kfree (vcpu_ctx);
}

void vcpu_init (void *info)
{
   int this_cpu = cur_logical_cpu ();
   
   if (!vcpu_supports_vmx ())
   {
      VCPU_DBG ("vmx not supported");
      return;
   }

   if (!vcpu_enable_vmx ())
   {
      VCPU_DBG ("failed to enable vmx operations");
      return;
   }

   vcpu_ctx_t *vcpu_ctx = g_vmm_ctx->vcpu_ctxs[this_cpu]; 
   if (vmxon (vcpu_ctx->vmxon_physical) != 0)
   {
      VCPU_DBG ("[vmxon] failed");
      vmxoff ();
      return;
   }
   VCPU_DBG ("[vmxon] executed successfully");

   if (!vmcs_setup (vcpu_ctx))
   {
      vmxoff ();
      return;
   }

   atomic_inc (&g_vmm_ctx->vcpu_init);
}

void vcpu_restore (void *info)
{
   vmxoff ();
   VCPU_DBG ("[vmxoff] executed");
}
