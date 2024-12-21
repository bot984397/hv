#define VCPU_DBG(fmt, ...) \
   LOG_DBG ("cpu %02d - " fmt, cur_logical_cpu (), ##__VA_ARGS__)

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
#include "asm.h"
#include "enc.h"
#include "vmcs.h"
#include "common.h"

// linux kernel devs try to maintain consistency challenge: impossible
#define __read_cr0 read_cr0
#define __write_cr0 write_cr0

// MOVE THIS TO A DIFFERENT FILE //////////////////////////////////////////////
typedef struct
{
   u64 code;
   const char *message;
} vmx_error_t;

static const vmx_error_t vmx_error_map [] = {
   { 1,  "VMCALL executed in VMX root operation" },
   { 2,  "VMCLEAR with invalid physical address" },
   { 3,  "VMCLEAR with VMXON pointer" },
   { 4,  "VMLAUNCH with non-clear VMCS" },
   { 5,  "VMRESUME with non-launched VMCS" },
   { 6,  "VMRESUME after VMXOFF (VMXOFF and VMXON between VMLAUNCH and VMRESUME)" },
   { 7,  "VM entry with invalid control field(s)" },
   { 8,  "VM entry with invalid host-state field(s)" },
   { 9,  "VMPTRLD with invalid physical address" },
   { 10, "VMPTRLD with VMXON pointer" },
   { 11, "VMPTRLD with incorrect VMCS revision identifier" },
   { 12, "VMREAD / VMWRITE from / to unsupported VMCS component" },
   { 13, "VMWRITE to read-only VMCS component" },
   { 15, "VMXON executed in VMX root operation" },
   { 16, "VM entry with invalid executive-VMCS pointer" },
   { 17, "VM entry with non-launched executive VMCS" },
   { 18, "VM entry with executive-VMCS pointer not VMXON pointer" },
   { 19, "VMCALL with non-clear VMCS (dual-monitor treatment)" },
   { 20, "VMCALL with invalid VM-exit control fields" },
   { 22, "VMCALL with incorrect MSEG revision identifier (dual-monitor treatment)" },
   { 23, "VMXOFF under dual-monitor treatment of SMIs and SMM" },
   { 24, "VMCALL with invalid SMM-monitor features (dual-monitor treatment)" },
   { 25, "VM entry with invalid VM-execution control fields in executive VMCS" },
   { 26, "VM entry with events blocked by MOV SS" },
   { 28, "Invalid operand to INVEPT / INVVPID" }
};
static const int vmx_error_map_size = (sizeof (vmx_error_map) / 
                                      sizeof (vmx_error_map[0]));

static const char* get_vmx_error_message (u64 code)
{
   for (u64 i = 0; i < vmx_error_map_size; i++)
   {
      if (vmx_error_map[i].code == code)
      {
         return vmx_error_map[i].message;
      }
   }
   return "Unknown error";
}

static inline __attribute__((always_inline)) const char* get_vmx_error (void)
{
   return get_vmx_error_message (__vmx_vmread (VMCS_RO_VM_INSTRUCTION_ERROR));
}

vcpu_ctx_t* vcpu_ctx_from_cpu_num (u32 cpu_num)
{
   vcpu_ctx_t *vcpu_ctx = g_vmm_ctx->vcpu_ctx_ll;
   while (vcpu_ctx)
   {
      if (vcpu_ctx->cpu_num == cpu_num)
      {
         return vcpu_ctx;
      }
      vcpu_ctx = vcpu_ctx->flink;
   }
   return NULL;
}

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

   cr4.value = __read_cr4 ();
   cr4.flags.VMXE = 1;
   __write_cr4 (cr4.value);

   cr4.value = __read_cr4 ();
   fixed.ctl = __rdmsr (IA32_VMX_CR4_FIXED0_MSR);
   cr4.value |= fixed.split.low;
   fixed.ctl = __rdmsr (IA32_VMX_CR4_FIXED1_MSR);
   cr4.value &= fixed.split.low;
   __write_cr4 (cr4.value);

   cr0.value = __read_cr0 ();
   fixed.ctl = __rdmsr (IA32_VMX_CR0_FIXED0_MSR);
   cr0.value |= fixed.split.low;
   fixed.ctl = __rdmsr (IA32_VMX_CR0_FIXED1_MSR);
   cr0.value &= fixed.split.low;
   __write_cr0 (cr0.value);

   return true;
}

static inline __attribute__((always_inline)) 
void vcpu_set_rev_ident (vcpu_ctx_t *vcpu_ctx)
{
   vcpu_ctx->vmxon_region->header.rev_ident =
      vcpu_ctx->cached.vmx_basic.fields.vmcs_rev_ident;
   vcpu_ctx->vmxon_region->header.reserved_0 = 0;
   vcpu_ctx->vmcs_region->header.rev_ident =
      vcpu_ctx->cached.vmx_basic.fields.vmcs_rev_ident;
   vcpu_ctx->vmcs_region->header.reserved_0 = 0;
}

__attribute__((warn_unused_result)) vcpu_ctx_t* vcpu_alloc (void)
{
   vcpu_ctx_t *vcpu_ctx;

   vcpu_ctx = kmalloc (sizeof (vcpu_ctx_t), GFP_KERNEL);
   if (vcpu_ctx == NULL)
   {
      goto FAILURE;
   }

   vcpu_ctx->vmxon_region = (vm_region_t *)mem_alloc_pages (0);
   if (vcpu_ctx->vmxon_region == NULL)
   {
      goto FAILURE;
   }
   vcpu_ctx->vmxon_physical = mem_virt_to_phys (vcpu_ctx->vmxon_region);

   vcpu_ctx->vmcs_region = (vm_region_t *)mem_alloc_pages (0);
   if (vcpu_ctx->vmcs_region == NULL)
   {
      goto FAILURE;
   }
   vcpu_ctx->vmcs_physical = mem_virt_to_phys (vcpu_ctx->vmcs_region);

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

   vcpu_ctx->bitmaps.msr_bitmaps = (u8 *)mem_alloc_pages (2);
   if (vcpu_ctx->bitmaps.msr_bitmaps == NULL)
   {
      goto FAILURE;
   }
   vcpu_ctx->bitmaps.msr_bitmaps_phys
      = mem_virt_to_phys (vcpu_ctx->bitmaps.msr_bitmaps);

   vcpu_ctx->cached.vmx_basic.value = __rdmsr (IA32_VMX_BASIC_MSR);
   vcpu_set_rev_ident (vcpu_ctx);

   return vcpu_ctx;

FAILURE:
   if (vcpu_ctx->vmxon_region)
   {
      mem_free_pages ((unsigned long)vcpu_ctx->vmxon_region, 0);
   }
   if (vcpu_ctx->vmcs_region)
   {
      mem_free_pages ((unsigned long)vcpu_ctx->vmcs_region, 0);
   }
   if (vcpu_ctx->bitmaps.io_bitmap_a)
   {
      mem_free_pages ((unsigned long)vcpu_ctx->bitmaps.io_bitmap_a, 0);
   }
   if (vcpu_ctx->bitmaps.io_bitmap_b)
   {
      mem_free_pages ((unsigned long)vcpu_ctx->bitmaps.io_bitmap_b, 0);
   }
   if (vcpu_ctx->bitmaps.msr_bitmaps)
   {
      mem_free_pages ((unsigned long)vcpu_ctx->bitmaps.msr_bitmaps, 2);
   }
   if (vcpu_ctx)
   {
      kfree (vcpu_ctx);
   }
   return NULL;
}

void vcpu_free (vcpu_ctx_t *vcpu_ctx)
{
   if (!vcpu_ctx)
   {
      return;
   }

   mem_free_pages_s ((unsigned long)vcpu_ctx->vmxon_region, 0);
   mem_free_pages_s ((unsigned long)vcpu_ctx->vmcs_region, 0);
   
   mem_free_pages_s ((unsigned long)vcpu_ctx->bitmaps.io_bitmap_a, 0);
   mem_free_pages_s ((unsigned long)vcpu_ctx->bitmaps.io_bitmap_b, 0);
   mem_free_pages_s ((unsigned long)vcpu_ctx->bitmaps.msr_bitmaps, 2);

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
   int err = __vmx_on (vcpu_ctx->vmxon_physical);
   if (err != 0)
   {
      VCPU_DBG ("[vmxon] failed");
      return;
   }
   VCPU_DBG ("[vmxon] executed successfully");

   if (!vmcs_setup (vcpu_ctx))
   {
      return;
   }

   atomic_inc (&g_vmm_ctx->vcpu_init);
}

void vcpu_restore (void *info)
{
   __vmx_off ();
   VCPU_DBG ("[vmxoff] executed");
}
