#ifndef __LKM_VMCS_H__
#define __LKM_VMCS_H__

#include "common.h"

__attribute__((warn_unused_result)) int vmcs_setup (vcpu_ctx_t *vcpu_ctx);

typedef union
{
   u64 value;
   struct
   {
      u64 external_interrupt_exiting      : 1;
      u64 reserved_0                      : 2;
      u64 nmi_exiting                     : 1;
      u64 reserved_1                      : 1;
      u64 virtual_nmis                    : 1;
      u64 activate_vmx_preemption_timer   : 1;
      u64 process_posted_interrupts       : 1;
      u64 reserved_2                      : 56;
   } fields;
} __vmx_pinbased_controls;

typedef union
{
   u64 value;
   struct
   {
      u64 reserved_0                   : 2;
      u64 interrupt_window_exiting     : 1;
      u64 use_tsc_offsetting           : 1;
      u64 reserved_1                   : 3;
      u64 hlt_exiting                  : 1;
      u64 reserved_2                   : 1;
      u64 invlpg_exiting               : 1;
      u64 mwait_exiting                : 1;
      u64 rdpmc_exiting                : 1;
      u64 rdtsc_exiting                : 1;
      u64 reserved_3                   : 2;
      u64 cr3_load_exiting             : 1;
      u64 cr3_store_exiting            : 1;
      u64 activate_tertiary_controls   : 1;
      u64 reserved_4                   : 1;
      u64 cr8_load_exiting             : 1;
      u64 cr8_store_exiting            : 1;
      u64 use_tpr_shadow               : 1;
      u64 nmi_window_exiting           : 1;
      u64 mov_dr_exiting               : 1;
      u64 unconditional_io_exiting     : 1;
      u64 use_io_bitmaps               : 1;
      u64 reserved_5                   : 1;
      u64 monitor_trap_flag            : 1;
      u64 use_msr_bitmaps              : 1;
      u64 monitor_exiting              : 1;
      u64 pause_exiting                : 1;
      u64 activate_secondary_controls  : 1;
      u64 reserved_6                   : 32;
   } fields;
} __vmx_primary_procbased_controls;

typedef union
{
   u64 value;
   struct
   {
      u64 virtualize_apic_accesses        : 1;
      u64 enable_ept                      : 1;
      u64 descriptor_table_exiting        : 1;
      u64 enable_rdtscp                   : 1;
      u64 virtualize_x2apic_mode          : 1;
      u64 enable_vpid                     : 1;
      u64 wbinvd_exiting                  : 1;
      u64 unrestricted_guest              : 1;
      u64 apic_register_virtualization    : 1;
      u64 virtual_interrupt_delivery      : 1;
      u64 pause_loop_exiting              : 1;
      u64 rdrand_exiting                  : 1;
      u64 enable_invpcid                  : 1;
      u64 enable_vm_functions             : 1;
      u64 vmcs_shadowing                  : 1;
      u64 enable_encls_exiting            : 1;
      u64 rdseed_exiting                  : 1;
      u64 enable_pml                      : 1;
      u64 ept_violation_ve                : 1;
      u64 conceal_vmx_from_pt             : 1;
      u64 enable_xsaves_xrstors           : 1;
      u64 pasid_translation               : 1;
      u64 mode_based_ept_execute          : 1;
      u64 ept_sub_page_write_permissions  : 1;
      u64 intel_pt_use_guest_phys_addr    : 1;
      u64 use_tsc_scaling                 : 1;
      u64 enable_user_wait_and_pause      : 1;
      u64 enable_pconfig                  : 1;
      u64 enable_enclv_exiting            : 1;
      u64 reserved_0                      : 1;
      u64 vmm_bus_lock_detection          : 1;
      u64 instruction_timeout             : 1;
      u64 reserved_1                      : 32;
   } fields;
} __vmx_secondary_procbased_controls;

typedef union
{
   u64 value;
   struct
   {
      
   } fields;
} __vmx_tertiary_procbased_controls;

#endif // __LKM_VMCS_H__
