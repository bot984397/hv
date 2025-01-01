#ifndef __LKM_ARCH_H__
#define __LKM_ARCH_H__

#include <linux/types.h>

#include "common.h"

// control registers

typedef union
{
   u64 ctl;
   struct
   {
      u64 PE         : 1;
      u64 MP         : 1;
      u64 EM         : 1;
      u64 TS         : 1;
      u64 ET         : 1;
      u64 NE         : 1;
      u64 reserved_0 : 10;
      u64 WP         : 1;
      u64 reserved_1 : 1;
      u64 AM         : 1;
      u64 reserved_2 : 11;
      u64 NW         : 1;
      u64 CD         : 1;
      u64 PG         : 1;
   };
} _cr0;

typedef union
{
   u64 ctl;
   struct
   {

   };
} _cr3;

typedef union
{
   u64 ctl;
   struct
   {
      u64 VME        : 1;
      u64 PVI        : 1;
      u64 TSD        : 1;
      u64 DE         : 1;
      u64 PSE        : 1;
      u64 PAE        : 1;
      u64 MCE        : 1;
      u64 PGE        : 1;
      u64 PCE        : 1;
      u64 OSFXR      : 1;
      u64 OSXMMEXCPT : 1;
      u64 UMIP       : 1;
      u64 LA57       : 1;
      u64 VMXE       : 1;
      u64 SMXE       : 1;
      u64 reserved_0 : 1;
      u64 FSGSBASE   : 1;
      u64 PCIDE      : 1;
      u64 OSXSAVE    : 1;
      u64 KL         : 1;
      u64 SMEP       : 1;
      u64 SMAP       : 1;
      u64 PKE        : 1;
      u64 CET        : 1;
      u64 PKS        : 1;
      u64 UINTR      : 1;
      u64 reserved_1 : 2;
      u64 LAM_SUP    : 1;
   };
} _cr4;

// FLAGS - EFLAGS - RFLAGS

typedef union
{
   u64 ctl;
   struct
   {
      u64 CF         : 1;
      u64 reserved_0 : 1;
      u64 PF         : 1;
      u64 reserved_1 : 1;
      u64 AF         : 1;
      u64 reserved_2 : 1;
      u64 ZF         : 1;
      u64 SF         : 1;
      u64 TF         : 1;
      u64 IF         : 1;
      u64 DF         : 1;
      u64 OF         : 1;
      u64 IOPL       : 2;
      u64 NT         : 1;
      u64 reserved_3 : 1;
      u64 RF         : 1;
      u64 VM         : 1;
      u64 AC         : 1;
      u64 VIF        : 1;
      u64 VIP        : 1;
      u64 ID         : 1;
      u64 reserved_4 : 42;
   };
} _rflags;

// MSRs

typedef union
{
   u64 ctl;
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
   };
} ia32_feature_control;
size_assert (ia32_feature_control, BITS (64));
#define IA32_FEATURE_CONTROL_MSR 0x0000003A

typedef union
{
   u64 ctl;
   struct
   {
      u64 vmcs_rev_ident     : 31;
      u64 reserved_0         : 1;
      u64 vmx_region_size    : 13;
      u64 reserved_1         : 3;
      u64 phys_addr_width    : 1;
      u64 dual_monitor_smm   : 1;
      u64 vmcs_memory_type   : 4;
      u64 vm_exit_reporting  : 1;
      u64 vmx_cap_support    : 1;
      u64 vm_entry_soft_he   : 1;
      u64 reserved_2         : 7;
   };
} ia32_vmx_basic;
size_assert (ia32_vmx_basic, BITS (64));
#define IA32_VMX_BASIC_MSR 0x00000480

typedef union
{
   u64 ctl;
   struct
   {
      u64 SCE        : 1;
      u64 reserved_0 : 7;
      u64 LME        : 1;
      u64 reserved_1 : 1;
      u64 LMA        : 1;
      u64 NXE        : 1;
      u64 reserved_2 : 52;
   };
} ia32_efer;
size_assert (ia32_efer, BITS (64));
#define IA32_EFER_MSR 0xC0000080

typedef union
{
   u64 ctl;
   struct
   {
      u64 LBR                    : 1;
      u64 BTF                    : 1;
      u64 BLD                    : 1;
      u64 reserved_0             : 3;
      u64 TR                     : 1;
      u64 BTS                    : 1;
      u64 BTINT                  : 1;
      u64 BTS_OFF_OS             : 1;
      u64 BTS_OFF_USR            : 1;
      u64 FREEZE_LBRS_ON_PMI     : 1;
      u64 FREEZE_PERFMON_ON_PMI  : 1;
      u64 ENABLE_UNCORE_PMI      : 1;
      u64 FREEZE_WHILE_SMM       : 1;
      u64 RTM_DEBUG              : 1;
      u64 reserved_1             : 48;
   };
} ia32_debugctl;
size_assert (ia32_debugctl, BITS (64));
#define IA32_DEBUGCTL_MSR 0x000001D9

typedef union
{
   u64 ctl;
   struct
   {
      u64 EN                  : 1;
      u64 BNDPRESERVE         : 1;
      u64 reserved_0          : 10;
      u64 bound_dir_base_addr : 52;
   };
} ia32_bndfcgs;
size_assert (ia32_bndfcgs, BITS (64));
#define IA32_BNDCFGS_MSR 0x00000D90

#define IA32_SYSENTER_CS_MSR              0x00000174
#define IA32_SYSENTER_ESP_MSR             0x00000175
#define IA32_SYSENTER_EIP_MSR             0x00000176
#define IA32_PERF_GLOBAL_CTRL_MSR         0x0000038F
#define IA32_PAT_MSR                      0x00000277
#define IA32_S_CET_MSR                    0x000006A2
#define IA32_ISSPT_ADDR_MSR               0x000006A8
#define IA32_PKRS_MSR                     0x000006E1
#define IA32_RTIT_CTL_MSR                 0x00000570
#define IA32_LBR_CTL_MSR                  0x000014CE

// Capability MSRs

typedef union
{
   u64 ctl;
   struct
   {
      u64 low;
      s64 high;
   };
} fixed_msr;

#define IA32_VMX_CR0_FIXED0_MSR           0x00000486
#define IA32_VMX_CR0_FIXED1_MSR           0x00000487
#define IA32_VMX_CR4_FIXED0_MSR           0x00000488
#define IA32_VMX_CR4_FIXED1_MSR           0x00000489

typedef union
{
   u64 ctl;
   struct
   {
      u32 _0;
      u32 _1;
   };
} cap_msr;

#define IA32_VMX_PINBASED_CTLS_MSR        0x00000481
#define IA32_VMX_TRUE_PINBASED_CTLS_MSR   0x0000048D
#define IA32_VMX_PROCBASED_CTLS_MSR       0x00000482
#define IA32_VMX_TRUE_PROCBASED_CTLS_MSR  0x0000048E
#define IA32_VMX_PROCBASED_CTLS2_MSR      0x0000048B
#define IA32_VMX_PROCBASED_CTLS3_MSR      0x00000492
#define IA32_VMX_EXIT_CTLS_MSR            0x00000483
#define IA32_VMX_TRUE_EXIT_CTLS_MSR       0x0000048F
#define IA32_VMX_EXIT_CTLS2_MSR           0x00000493
#define IA32_VMX_ENTRY_CTLS_MSR           0x00000484
#define IA32_VMX_TRUE_ENTRY_CTLS_MSR      0x00000490

#endif // __LKM_ARCH_H__
