#ifndef __LKM_VMCS_H__
#define __LKM_VMCS_H__

#include "common.h"

#define VMCS_HOST_SELECTOR_MASK 0xF8

#define TSS_AVAILABLE 0x9
#define TSS_BUSY 0xB

__attribute__((warn_unused_result)) int vmcs_setup (cpu_ctx *vcpu_ctx);

typedef union
{
   u32 ctl;
   struct
   {
      u32 external_interrupt_exiting      : 1;
      u32 reserved_0                      : 2;
      u32 nmi_exiting                     : 1;
      u32 reserved_1                      : 1;
      u32 virtual_nmis                    : 1;
      u32 activate_vmx_preemption_timer   : 1;
      u32 process_posted_interrupts       : 1;
      u32 reserved_2                      : 24;
   };
} __vmx_pinbased_controls;
size_assert (__vmx_pinbased_controls, BITS(32));

typedef union
{
   u32 ctl;
   struct
   {
      u32 reserved_0                   : 2;
      u32 interrupt_window_exiting     : 1;
      u32 use_tsc_offsetting           : 1;
      u32 reserved_1                   : 3;
      u32 hlt_exiting                  : 1;
      u32 reserved_2                   : 1;
      u32 invlpg_exiting               : 1;
      u32 mwait_exiting                : 1;
      u32 rdpmc_exiting                : 1;
      u32 rdtsc_exiting                : 1;
      u32 reserved_3                   : 2;
      u32 cr3_load_exiting             : 1;
      u32 cr3_store_exiting            : 1;
      u32 activate_tertiary_controls   : 1;
      u32 reserved_4                   : 1;
      u32 cr8_load_exiting             : 1;
      u32 cr8_store_exiting            : 1;
      u32 use_tpr_shadow               : 1;
      u32 nmi_window_exiting           : 1;
      u32 mov_dr_exiting               : 1;
      u32 unconditional_io_exiting     : 1;
      u32 use_io_bitmaps               : 1;
      u32 reserved_5                   : 1;
      u32 monitor_trap_flag            : 1;
      u32 use_msr_bitmaps              : 1;
      u32 monitor_exiting              : 1;
      u32 pause_exiting                : 1;
      u32 activate_secondary_controls  : 1;
   };
} __vmx_procbased_ctls;
size_assert (__vmx_procbased_ctls, BITS(32));

typedef union
{
   u32 ctl;
   struct
   {
      u32 virtualize_apic_accesses        : 1;
      u32 enable_ept                      : 1;
      u32 descriptor_table_exiting        : 1;
      u32 enable_rdtscp                   : 1;
      u32 virtualize_x2apic_mode          : 1;
      u32 enable_vpid                     : 1;
      u32 wbinvd_exiting                  : 1;
      u32 unrestricted_guest              : 1;
      u32 apic_register_virtualization    : 1;
      u32 virtual_interrupt_delivery      : 1;
      u32 pause_loop_exiting              : 1;
      u32 rdrand_exiting                  : 1;
      u32 enable_invpcid                  : 1;
      u32 enable_vm_functions             : 1;
      u32 vmcs_shadowing                  : 1;
      u32 enable_encls_exiting            : 1;
      u32 rdseed_exiting                  : 1;
      u32 enable_pml                      : 1;
      u32 ept_violation_ve                : 1;
      u32 conceal_vmx_from_pt             : 1;
      u32 enable_xsaves_xrstors           : 1;
      u32 pasid_translation               : 1;
      u32 mode_based_ept_execute          : 1;
      u32 ept_sub_page_write_permissions  : 1;
      u32 intel_pt_use_guest_phys_addr    : 1;
      u32 use_tsc_scaling                 : 1;
      u32 enable_user_wait_and_pause      : 1;
      u32 enable_pconfig                  : 1;
      u32 enable_enclv_exiting            : 1;
      u32 reserved_0                      : 1;
      u32 vmm_bus_lock_detection          : 1;
      u32 instruction_timeout             : 1;
   };
} __vmx_procbased_ctls2;
size_assert (__vmx_procbased_ctls2, BITS(32));

typedef union
{
   u64 ctl;
   struct
   {
      u64 loadiwkey_exiting            : 1;
      u64 enable_hlat                  : 1;
      u64 ept_paging_write_control     : 1;
      u64 guest_paging_verification    : 1;
      u64 ipi_virtualization           : 1;
      u64 reserved_0                   : 1;
      u64 enable_msr_list_instructions : 1;
      u64 virtualize_ia32_spec_ctrl    : 1;
      u64 reserved_1                   : 56;
   };
} __vmx_procbased_ctls3;
size_assert (__vmx_procbased_ctls3, BITS(64));

typedef union
{
   u32 ctl;
   struct
   {
      u32 reserved_0                      : 2;
      u32 save_debug_controls             : 1;
      u32 reserved_1                      : 6;
      u32 host_address_space_size         : 1;
      u32 reserved_2                      : 2;
      u32 load_ia32_perf_global_ctrl      : 1;
      u32 reserved_3                      : 2;
      u32 acknowledge_interrupt_on_exit   : 1;
      u32 reserved_4                      : 2;
      u32 save_ia32_pat                   : 1;
      u32 load_ia32_pat                   : 1;
      u32 save_ia32_efer                  : 1;
      u32 load_ia32_efer                  : 1;
      u32 save_vmx_preemption_timer_value : 1;
      u32 clear_ia32_bndcfgs              : 1;
      u32 conceal_vmx_from_pt             : 1;
      u32 clear_ia32_rtit_ctl             : 1;
      u32 clear_ia32_lbr_ctl              : 1;
      u32 clear_uinv                      : 1;
      u32 load_cet_state                  : 1;
      u32 load_pkrs                       : 1;
      u32 save_ia32_perf_global_ctl       : 1;
      u32 activate_secondary_controls     : 1;
   };
} __vmx_exit_ctls;
size_assert (__vmx_exit_ctls, BITS(32));

typedef union
{
   u32 ctl;
   struct
   {
      u32 reserved_0                      : 3;
      u32 prematurely_busy_shadow_stack   : 1;
      u32 reserved_1                      : 28;
   };
} __vmx_exit_ctls2;
size_assert (__vmx_exit_ctls2, BITS(32));

typedef union
{
   u32 ctl;
   struct
   {
      u32 reserved_0                         : 2;
      u32 load_debug_controls                : 1;
      u32 reserved_1                         : 6;
      u32 ia32e_mode_guest                   : 1;
      u32 entry_to_smm                       : 1;
      u32 deactivate_dual_monitor_treatment  : 1;
      u32 reserved_2                         : 1;
      u32 load_ia32_perf_global_ctrl         : 1;
      u32 load_ia32_pat                      : 1;
      u32 load_ia32_efer                     : 1;
      u32 load_ia32_bndcfgs                  : 1;
      u32 conceal_vmx_from_pt                : 1;
      u32 load_ia32_rtit_ctl                 : 1;
      u32 load_uinv                          : 1;
      u32 load_cet_state                     : 1;
      u32 load_guest_ia32_lbr_ctl            : 1;
      u32 load_pkrs                          : 1;
      u32 reserved_3                         : 9;
   };
} __vmx_entry_ctls;
size_assert (__vmx_entry_ctls, BITS(32));

typedef union
{
   u32 ctl;
   struct
   {
      u32 divide_error                    : 1;
      u32 debug                           : 1;
      u32 nmi_interrupt                   : 1;
      u32 breakpoint                      : 1;
      u32 overflow                        : 1;
      u32 bound_range_exceeded            : 1;
      u32 invalid_opcode                  : 1;
      u32 device_not_available            : 1;
      u32 double_fault                    : 1;
      u32 coprocessor_segment_overrun     : 1;
      u32 invalid_tss                     : 1;
      u32 segment_not_present             : 1;
      u32 stack_segment_fault             : 1;
      u32 general_protection              : 1;
      u32 page_fault                      : 1;
      u32 reserved_1                      : 1;
      u32 floating_point_error            : 1;
      u32 alignment_check                 : 1;
      u32 machine_check                   : 1;
      u32 simd_floating_point_exception   : 1;
      u32 virtualization_exception        : 1;
      u32 control_protection_exception    : 1;
      u32 reserved_2                      : 10;
   };
} __vmx_exception_bitmap;
size_assert (__vmx_exception_bitmap, BITS(32));

typedef union
{
   u16 ctl;
   struct
   {
      u16 RPL              : 2;
      u16 table_indicator  : 1;
      u16 index            : 13;
   };
} __segment_selector;

typedef struct
{
   u16 limit_low;
   u16 base_low;
   union
   {
      u32 ctl;
      struct
      {
         u32 base_mid               : 8;
         u32 segment_type           : 4;
         u32 descriptor_type        : 1;
         u32 DPL                    : 2;
         u32 present                : 1;
         u32 limit_high             : 4;
         u32 available_for_system   : 1;
         u32 long_mode              : 1;
         u32 DB                     : 1;
         u32 granularity            : 1;
         u32 base_high              : 8;
      };
   };
} __segment_descriptor_32;
size_assert (__segment_descriptor_32, BITS(64));

typedef struct
{
   u16 limit_low;
   u16 base_low;
   union
   {
      u32 ctl0;
      struct
      {
         u32 base_mid               : 8;
         u32 segment_type           : 4;
         u32 reserved_0             : 1;
         u32 DPL                    : 2;
         u32 present                : 1;
         u32 limit_high             : 4;
         u32 available_for_system   : 1;
         u32 reserved_1             : 1;
         u32 reserved_2             : 1;
         u32 granularity            : 1;
         u32 base_high              : 8;
      };
   };
   u32 base_upper;
   union
   {
      u32 ctl1;
      struct
      {
         u32 reserved_3 : 8;
         u32 reserved_4 : 5;
         u32 reserved_5 : 19;
      };
   };
} __segment_descriptor_64;
size_assert (__segment_descriptor_64, BITS(128));

typedef union
{
   u32 ctl;
   struct
   {
      u32 segment_type     : 4;
      u32 descriptor_type  : 1;
      u32 DPL              : 2;
      u32 segment_present  : 1;
      u32 reserved_0       : 4;
      u32 AVL              : 1;
      u32 long_mode        : 1;
      u32 DB               : 1;
      u32 granularity      : 1;
      u32 segment_unusable : 1;
      u32 reserved_1       : 15;
   };
} __segment_access_rights;
size_assert (__segment_access_rights, BITS(32));

typedef struct pseudo_descriptor __pseudo_descriptor;
struct __attribute__((packed)) pseudo_descriptor
{
   u16 limit;
   u64 base;
};
size_assert (__pseudo_descriptor, BITS(80));

#endif // __LKM_VMCS_H__
