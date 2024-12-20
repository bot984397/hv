#ifndef __LKM_VMCS_H__
#define __LKM_VMCS_H__

#include "common.h"

__attribute__((warn_unused_result)) int vmcs_setup (vcpu_ctx_t *vcpu_ctx);

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
   } bits;
} __vmx_procbased_ctls;

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
      u32 reserved_1                      : 32;
   } bits;
} __vmx_procbased_ctls2;

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

#endif // __LKM_VMCS_H__
