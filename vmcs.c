#include <linux/types.h>
#include <asm/msr.h>

#include "vmcs.h"
#include "asm.h"
#include "msr.h"
#include "enc.h"
#include "common.h"

static void vmcs_adjust_controls (u32 *ctl, u32 cap)
{
   ia32_generic_cap_msr cap_msr = {0};
   cap_msr.value = __rdmsr (cap);

   *ctl |= cap_msr.split.allowed_0;
   *ctl &= cap_msr.split.allowed_1;
}

static __vmx_pinbased_controls vmcs_setup_pinbased_ctls (void)
{
   __vmx_pinbased_controls control = {0};

   /*
    * true: external interrupts cause vm exits
    * false: external interrupts delivered through guest IDT
    */
   control.external_interrupt_exiting = false; 

   /*
    * true: NMIs cause vm exits
    * false: NMIs delivered via descriptor 2 of the IDT
    */
   control.nmi_exiting = false;

   /* 
    * true: NMIs are never blocked
    */
   control.virtual_nmis = false;

   /*
    * true: vmx preemption timer enabled
    * false: vmx preemption timer disabled
    */
   control.activate_vmx_preemption_timer = false;

   /*
    * true: interrupts with posted-interrupt notification vector are treated
    *       specially, updating virtual-APIC page with PI requests
    */
   control.process_posted_interrupts = false;

   return control;
}

static __vmx_procbased_ctls vmcs_setup_primary_procbased_ctls (void)
{
   __vmx_procbased_ctls control = {0};

   

   return control;
}

static __vmx_procbased_ctls2 vmcs_setup_secondary_procbased_ctls (void)
{
   __vmx_procbased_ctls2 control = {0};



   return control;
}

static __vmx_exit_ctls vmcs_setup_primary_exit_ctls (void)
{
   __vmx_exit_ctls control = {0};



   return control;
}

static void vmcs_setup_control (vcpu_ctx_t *vcpu_ctx)
{
   __vmx_pinbased_controls pinbased_ctl;
   __vmx_procbased_ctls procbased_ctl;
   __vmx_procbased_ctls2 procbased_ctl2;

   u8 true_controls = vcpu_ctx->cached.vmx_basic.fields.vmx_cap_support;

   pinbased_ctl = vmcs_setup_pinbased_ctls ();
   vmcs_adjust_controls (&pinbased_ctl.ctl, true_controls == 1
         ? IA32_VMX_PINBASED_CTLS_MSR
         : IA32_VMX_TRUE_PINBASED_CTLS_MSR);

   procbased_ctl = vmcs_setup_primary_procbased_ctls ();
   vmcs_adjust_controls (&procbased_ctl.ctl, true_controls == 1
         ? IA32_VMX_PROCBASED_CTLS_MSR
         : IA32_VMX_TRUE_PROCBASED_CTLS_MSR);

   procbased_ctl2 = vmcs_setup_secondary_procbased_ctls ();
   vmcs_adjust_controls (&procbased_ctl2.ctl, IA32_VMX_PROCBASED_CTLS2_MSR);

   __vmx_vmwrite (VMCS_CTRL_PINBASED_CONTROLS, pinbased_ctl.ctl);
   __vmx_vmwrite (VMCS_CTRL_PROCBASED_CTLS, procbased_ctl.ctl);
   __vmx_vmwrite (VMCS_CTRL_PROCBASED_CTLS2, procbased_ctl2.ctl);

   // no vm-exits on any exception
   __vmx_vmwrite (VMCS_CTRL_EXCEPTION_BITMAP, 0);

   __vmx_vmwrite (VMCS_CTRL_PAGE_FAULT_ERROR_CODE_MASK, 0);
   __vmx_vmwrite (VMCS_CTRL_PAGE_FAULT_ERROR_CODE_MATCH, 0);

   __vmx_vmwrite (VMCS_CTRL_CR3_TARGET_COUNT, 0);
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
   vmcs_setup_control (vcpu_ctx);

   return 0;
}
