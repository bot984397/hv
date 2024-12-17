#ifndef __LKM_MSR_H__
#define __LKM_MSR_H__

#include <linux/types.h>

typedef union
{
   u64 value;
   struct
   {
      u64 locked                    : 1;
      u64 vmx_inside_smx            : 1;
      u64 vmx_outside_smx           : 1;
      u64 reserved_0                : 5;
      u64 senter_local_enable       : 7;
      u64 senter_global_enable      : 1;
      u64 reserved_1                : 1;
      u64 sgx_launch_control_enable : 1;
      u64 sgx_global_enable         : 1;
      u64 reserved_2                : 1;
      u64 lmce_on                   : 1;
      u64 reserved_3                : 43;
   } fields;
} ia32_feature_control_t;
#define IA32_FEATURE_CONTROL_MSR 0x03A

typedef union
{
   u64 value;
   struct
   {
      u64 vmcs_rev_ident     : 31;
      u64 reserved_0         : 1;  // always 0
      u64 vmx_region_size    : 13;
      u64 reserved_1         : 3;  // always 0
      u64 phys_addr_width    : 1;
      u64 dual_monitor_smm   : 1;
      u64 vmcs_memory_type   : 4;
      u64 vm_exit_reporting  : 1;
      u64 vmx_cap_support    : 1;
      u64 vm_entry_soft_he   : 1;
      u64 reserved_2         : 7;
   } fields;
} ia32_vmx_basic_t;
#define IA32_VMX_BASIC_MSR 0x480

typedef union
{
   u32 value;
   struct
   {
      u32 LBR                    : 1;
      u32 BTF                    : 1;
      u32 BLD                    : 1;
      u32 reserved_0             : 3;
      u32 TR                     : 1;
      u32 BTS                    : 1;
      u32 BTINT                  : 1;
      u32 BTS_OFF_OS             : 1;
      u32 BTS_OFF_USR            : 1;
      u32 FREEZE_LBRS_ON_PMI     : 1;
      u32 FREEZE_PERFMON_ON_PMI  : 1;
      u32 reserved_1             : 1;
      u32 FREEZE_WHILE_SMM       : 1;
      u32 RTM                    : 1;
      u32 reserved_2             : 16;
   } fields;
} ia32_debugctl_t;
#define IA32_DEBUGCTL_MSR 0x1D9

#define IA32_SYSENTER_CS_MSR 0x174
#define IA32_SYSENTER_EIP_MSR 0x176
#define IA32_SYSENTER_ESP_MSR 0x175

typedef union
{
   u64 value;
   struct
   {
      u64 low;
      s64 high;
   } split;
} ia32_gen_fixed_t;
#define IA32_VMX_CR0_FIXED0_MSR 0x486
#define IA32_VMX_CR0_FIXED1_MSR 0x487
#define IA32_VMX_CR4_FIXED0_MSR 0x488
#define IA32_VMX_CR4_FIXED1_MSR 0x489

typedef union
{
   u64 value;
   struct
   {
      u32 allowed_0;
      u32 allowed_1;
   }
} ia32_generic_cap_msr;
#define IA32_VMX_PINBASED_CTLS_MSR 0x481
#define IA32_VMX_TRUE_PINBASED_CTLS_MSR 0x48D
#define IA32_VMX_PROCBASED_CTLS_MSR 0x482
#define IA32_VMX_TRUE_PROCBASED_CTLS_MSR 0x48E
#define IA32_VMX_PROCBASED_CTLS2_MSR 0x48B

// vmcs enumeration
#define IA32_VMX_VMCS_ENUM_MSR 0x48A

#endif // __LKM_MSR_H__
