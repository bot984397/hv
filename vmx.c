#include <linux/types.h>

#include "vmx.h"
#include "enc.h"
#include "intrin.h"

typedef struct
{
   u64 err;
   const char *msg;
} __vmx_err;

static const __vmx_err vmx_err_map [] = {
   { 1,  "VMCALL executed in VMX root operation" },
   { 2,  "VMCLEAR with invalid physical address" },
   { 3,  "VMCLEAR with VMXON pointer" },
   { 4,  "VMLAUNCH with non-clear VMCS" },
   { 5,  "VMRESUME with non-launched VMCS" },
   { 6,  "VMRESUME after VMXOFF" },
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
   { 22, "VMCALL with incorrect MSEG revision identifier" },
   { 23, "VMXOFF under dual-monitor treatment of SMIs and SMM" },
   { 24, "VMCALL with invalid SMM-monitor features (dual-monitor treatment)" },
   { 25, "VM entry with invalid VM-exec control fields in executive VMCS" },
   { 26, "VM entry with events blocked by MOV SS" },
   { 28, "Invalid operand to INVEPT / INVVPID" }
};
static const int vmx_err_map_sz = (sizeof (vmx_err_map) / sizeof (__vmx_err));

static const char* vmx_error_internal (u64 err)
{
   for (register u64 i = 0; i < vmx_err_map_sz; i++)
   {
      if (vmx_err_map[i].err == err)
      {
         return vmx_err_map[i].msg;
      }
   }
   return "Unknown error";
}

const char* vmx_get_error_message (void)
{
   return vmx_error_internal (vmread1 (VMCS_RO_VM_INSTRUCTION_ERROR));
}

int vmexit_handler (cpu_ctx *_cpu_ctx)
{
   LOG_DBG ("vm exit handler called");

   vm_exit_reason exit = { .ctl = vmread1 (VMCS_RO_EXIT_REASON) };
   if (exit.vm_entry_failure == 1)
      LOG_DBG ("vm entry failure");
   else
      LOG_DBG ("true vm exit");

   LOG_DBG ("exit reason: %d", exit.basic_exit_reason);
   LOG_DBG ("exit qualification: %llu", vmread1 (VMCS_RO_EXIT_QUALIFICATION));

   return 0;
}

_always_inline_ void vmresume_error_handler (void)
{
   LOG_DBG ("vmresume error handler called");
}
