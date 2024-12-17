#include <linux/types.h>
#include <asm/msr.h>

#include "vmcs.h"
#include "asm.h"
#include "msr.h"
#include "enc.h"
#include "common.h"

static u64 vmcs_adjust_controls (u32 cap)
{
   u64 final = 0;
   ia32_generic_cap_msr cap_msr = {0};

   cap_msr.value = __rdmsr (cap);

   final |= cap_msr.allowed_0;
   final &= cap_msr.allowed_1;

   return final;
}

static void vmcs_setup_pinbased_ctls (vcpu_ctx_t *vcpu_ctx)
{
   __vmx_pinbased_controls pinbased_ctl = {0};
   pinbased_ctl.value = vmcs_adjust_controls (
         vcpu_ctx->cached.vmx_basic.fields.vmx_cap_support == 1
         ? IA32_VMX_PINBASED_CTLS_MSR
         : IA32_VMX_TRUE_PINBASED_CTLS_MSR);

   __vmx_vmwrite (VMCS_CTRL_PINBASED_EXECUTION_CONTROLS, pinbased_ctl.value);
}

static void vmcs_setup_procbased_ctls (vcpu_ctx_t *vcpu_cts)
{
   __vmx_primary_procbased_controls primary_procbased_ctl = {0};
   __vmx_secondary_procbased_controls secondary_procbased_ctl = {0};
   __vmx_tertiary_procbased_controls tertiary_procbased_ctl = {0};

   primary_procbased_ctl.value = vmcs_adjust_controls (
         vcpu_ctx->cached.vmx_basic.fields.vmx_cap_support == 1
         ? IA32_VMX_PROCBASED_CTLS_MSR
         : IA32_VMX_TRUE_PROCBASED_CTLS_MSR);
   primary_procbased_ctl.fields.activate_secondary_controls = 1;
   primary_procbased_ctl.fields.activate_tertiary_controls = 1;
   primary_procbased_ctl.fields.use_io_bitmaps = 1;
   primary_procbased_ctl.fields.use_msr_bitmaps = 1;
   __vmx_vmwrite (VMCS_CTRL_PRIMARY_PROCBASED_CONTROLS,
                  primary_procbased_ctl.value);

   secondary_procbased_ctl.value = vmcs_adjust_controls (
         IA32_VMX_PROCBASED_CTLS2_MSR);
   secondary_procbased_ctl.fields.enable_vm_functions = 1;
   secondary_procbased_ctl.fields.vmcs_shadowing = 0;
   __vmx_vmwrite (VMCS_CTRL_SECONDARY_PROCBASED_CONTROLS,
                  secondary_procbased_ctl.value);
}

__attribute__((warn_unused_result))
static int vmcs_setup_control (vcpu_ctx_t *vcpu_ctx)
{
   __vmx_primary_procbased_controls primary_procbased_ctl = {0};
   __vmx_secondary_procbased_controls secondary_procbased_ctl = {0};

   primary_procbased_ctl.value = vmcs_adjust_controls (
         vcpu_ctx->cached.vmx_basic.fields.vmx_cap_support == 1
         ? IA32_VMX_PROCBASED_CTLS_MSR
         : IA32_VMX_TRUE_PROCBASED_CTLS_MSR);

   return 0;
}

__attribute__((warn_unused_result)) 
static int vmcs_setup_guest (vcpu_ctx_t *vcpu_ctx)
{
   return 0;
}

__attribute__((warn_unused_result))
static int vmcs_setup_host (vcpu_ctx_t *vcpu_ctx)
{
   return 0;
}

__attribute__((warn_unused_result)) 
int vmcs_setup (vcpu_ctx_t *vcpu_ctx)
{
   return 0;
}
