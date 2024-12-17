#include "vmcs.h"
#include "asm.h"

__attribute__((warn_unused_result))
static int vmcs_setup_control (vcpu_ctx_t *vcpu_ctx)
{
   ia32_vmx_basic_t basic_msr = {0};
   __vmx_pinbased_vm_execution_controls pinbased_ctl = {0};

   basic_msr.value = __rdmsr (IA32_VMX_BASIC_MSR);

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
