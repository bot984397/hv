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

   /*
    * true: vm exit occurs at the beginning of any instruction
    *       if RFLAGS.IF == 1 and there are no other interrupt blocks
    */
   control.interrupt_window_exiting = false;

   /*
    * true: use tsc offset field for RDTSC / RDTSCP / IA32_TIME_STAMP_COUNTER
    */
   control.use_tsc_offsetting = false;

   /*
    * true: executions of HLT cause vm exits
    */
   control.hlt_exiting = false;

   /*
    * true: executions of INVLPG cause vm exits
    */
   control.invlpg_exiting = false;

   /*
    * true: executions of MWAIT cause vm exits
    */
   control.mwait_exiting = false;

   /*
    * true: executions of RDPMC cause vm exits
    */
   control.rdpmc_exiting = false;

   /*
    * true: executions of RDTSC / RDTSCP cause vm exits
    */
   control.rdtsc_exiting = false;

   /*
    * true: MOV to CR3 causes a vm exit
    */
   control.cr3_load_exiting = false;

   /*
    * true: MOV from CR3 causes a vm exit
    */
   control.cr3_store_exiting = false;

   /*
    * true: activate tertiary processor based vm-execution controls
    */
   control.activate_tertiary_controls = false;

   /*
    * true: MOV to CR8 causes a vm exit
    */
   control.cr8_load_exiting = false;

   /*
    * true: MOV from CR8 causes a vm exit
    */
   control.cr8_store_exiting = false;

   /*
    * true: enables TPR virtualization and other APIC-virtualization features
    */
   control.use_tpr_shadow = false;

   /*
    * true: vm exit at the beginning of any instruction if there is no
    *       NMI blocking
    */
   control.nmi_window_exiting = false;

   /*
    * true: MOV DR execution causes a vm exit
    */
   control.mov_dr_exiting = false;

   /*
    * true: executions of I/O instructions cause vm exits
    */
   control.unconditional_io_exiting = false;

   /*
    * true: I/O bitmaps are used to restrict I/O instruction execution
    */
   control.use_io_bitmaps = false;

   /*
    * true: monitor trap flag debugging feature eneabled
    */
   control.monitor_trap_flag = false;

   /*
    * true: MSR bitmaps are used to control RDMSR / WRMSR execution
    */
   control.use_msr_bitmaps = true;

   /*
    * true: executions of MONITOR cause a vm exit
    */
   control.monitor_exiting = false;

   /*
    * true: executions of PAUSE cause a vm exit
    */
   control.pause_exiting = false;

   /*
    * true: activate secondary processor based vm-execution controls
    */
   control.activate_secondary_controls = true;

   return control;
}

static __vmx_procbased_ctls2 vmcs_setup_secondary_procbased_ctls (void)
{
   __vmx_procbased_ctls2 control = {0};

   /*
    * true: accesses to page with APIC-access address are treated specially
    */
   control.virtualize_apic_accesses = false;

   /*
    * true: Extended Page Tables (EPT) are enabled
    */
   control.enable_ept = true;

   /*
    * true: LGDT, LIDT, LLDT, LTR, SGDT, SIDT, SLDT and STR cause VM exits
    */
   control.descriptor_table_exiting = false;

   /*
    * false: execution of RDTSCP causes an invalid-opcode (#UD) exception
    */
   control.enable_rdtscp = true;

   /*
    * true: RDMSR and WRMSR from and to APIC MSRs are treated specially
    */
   control.virtualize_x2apic_mode = false;

   /*
    * true: cached translations of linear addresses are associated with VPID
    */
   control.enable_vpid = true;

   /*
    * true: executions of WBINVD cause VM exits
    */
   control.wbinvd_exiting = false;

   /*
    * true: guest may run in unpaged protected or real address mode
    */
   control.unrestricted_guest = false;

   /*
    * true: certain APIC accesses are virtualized
    */
   control.apic_register_virtualization = false;

   /*
    * true: evalutation and delivery of pending virtual interrupts enabled
    */
   control.virtual_interrupt_delivery = false;

   /*
    * true: series of PAUSE executions can cause a VM exit
    */
   control.pause_loop_exiting = false;

   /*
    * true: executions of RDRAND cause VM exits
    */
   control.rdrand_exiting = false;

   /*
    * false: execution of INVPCID causes an invalid-opcode (#UD) exception
    */
   control.enable_invpcid = true;

   /*
    * true: VMFUNC execution in VMX Non-Root operation enabled
    */
   control.enable_vm_functions = false;

   /*
    * true: VMREAD / VMWRITE in VMX Non-Root operation may access shadow VMCS
    */
   control.vmcs_shadowing = false;

   /*
    * true: executions of ENCLS consult the ENCLS-exiting bitmap to determine
    *       wether the instruction causes a VM exit
    */
   control.enable_encls_exiting = false;

   /*
    * true: executions of RDSEED cause VM exits
    */
   control.rdseed_exiting = false;

   /*
    * true: access to guest-physical address that sets EPT dirty bit first
    *       adds an entry to the page modification log (PML)
    */
   control.enable_pml = false;

   /*
    * true: EPT violations may cause virtualization exceptions (#VE) instead
    *       of VM exits
    */
   control.ept_violation_ve = false;

   /*
    * true: Intel PT suppresses from PIPs an indication that the processor
    *       was in VMX non-root operation and omits a VMCS packet from any
    *       PSB+ produced in VMX non-root operation
    */
   control.conceal_vmx_from_pt = true;

   /*
    * false: execution of XSAVES / XRSTORS causes an undefined-opcode
    *        exception (#UD)
    */
   control.enable_xsaves_xrstors = true;

   /*
    * true: EPT execute permissions are based on wether the linear address
    *       being accessed is supervisor or user mode
    */
   control.mode_based_ept_execute = false;

   /*
    * true:
    */
   control.ept_sub_page_write_permissions = false;

   /*
    * true:
    */
   control.intel_pt_use_guest_phys_addr = false;

   /*
    * true:
    */
   control.use_tsc_scaling = false;

   /*
    * true:
    */
   control.enable_user_wait_and_pause = false;

   /*
    * true:
    */
   control.enable_pconfig = false;

   /*
    * true:
    */
   control.enable_enclv_exiting = false;

   /*
    * true:
    */
   control.vmm_bus_lock_detection = false;

   /*
    * true:
    */
   control.instruction_timeout = false;

   return control;
}

static __vmx_exit_ctls vmcs_setup_primary_exit_ctls (void)
{
   __vmx_exit_ctls control = {0};

   /*
    * true: DR7 and IA32_DEBUGCTL MSR are saved on vm exit
    */
   control.save_debug_controls = true;

   /*
    * true: processor is in 64-bit mode after vm exit
    */
   control.host_address_space_size = true;

   /*
    * true: IA32_PERF_GLOBAL_CTRL MSR is loaded on vm exit
    */
   control.load_ia32_perf_global_ctrl = false;

   /*
    * true: if vm exit occurs due to external interrupt, processor
    *       acknowledges the interrupt controller, acquiring the interrupts
    *       vector. vector is stored in the interruption-information field
    */
   control.acknowledge_interrupt_on_exit = true;

   /*
    * true: IA32_PAT MSR is saved on vm exit
    */
   control.save_ia32_pat = false;

   /*
    * true: IA32_PAT MSR is loaded on vm exit
    */
   control.load_ia32_pat = false;

   /*
    * true: IA32_EFER MSR is saved on vm exit
    */
   control.save_ia32_efer = false;

   /*
    * true: IA32_EFER MSR is loaded on vm exit
    */
   control.load_ia32_efer = false;

   /*
    * true: vmx preemption timer value is saved on vm exit
    */
   control.save_vmx_preemption_timer_value = false;

   /*
    * true: IA32_BNDCFGS MSR is cleared on vm exit
    */
   control.clear_ia32_bndcfgs = false;

   /*
    * true: Intel PT does not produce a paging information packet on VM exit
    *       or a VMCS packet on SMM VM exit
    */
   control.conceal_vmx_from_pt = true;

   /*
    * true: IA32_RTIT_CTL MSR is cleared on VM exit
    */
   control.clear_ia32_rtit_ctl = false;

   /*
    * true: IA32_LBR_CTL MSR is cleared on VM exit
    */
   control.clear_ia32_lbr_ctl = false;

   /*
    * true: UINV is cleared on VM exit
    */
   control.clear_uinv = false;

   /*
    * true: CET-related MSRs and SPP are loaded on VM exit
    */
   control.load_cet_state = false;

   /*
    * true: PKRS is loaded on VM exit
    */
   control.load_pkrs = false;

   /*
    * true: IA32_PERF_GLOBAL_CTL is saved on VM exit
    */
   control.save_ia32_perf_global_ctl = false;

   /*
    * true: activate secondary VM-exit controls
    */
   control.activate_secondary_controls = false;

   return control;
}

static __vmx_entry_ctls vmcs_setup_entry_ctls (void)
{
   __vmx_entry_ctls control = {0};

   /*
    * true: DR7 and IA32_DEBUGCTL MSR are loaded on VM entry
    */
   control.load_debug_controls = true;

   /*
    * true: logical processor is in IA-32e mode after VM entry
    */
   control.ia32e_mode_guest = true;

   /*
    * true: logical processor is in SMM after VM entry
    */
   control.entry_to_smm = false;

   /*
    * true: SMIs and SMM get default treatment after VM entry
    */
   control.deactivate_dual_monitor_treatment = false;

   /*
    * true: IA32_PERF_GLOBAL_CTRL MSR is loaded on VM entry
    */
   control.load_ia32_perf_global_ctrl = false;

   /*
    * true: IA32_PAT MSR is loaded on VM entry
    */
   control.load_ia32_pat = false;

   /*
    * true: IA32_EFER MSR is loaded on VM entry
    */
   control.load_ia32_efer = false;

   /*
    * true: IA32_BNDCFGS MSR is loaded on VM entry
    */
   control.load_ia32_bndcfgs = false;

   /*
    * true: Intel PT does not produce a paging information packet on VM entry
    *       or a VMCS packet on VM entry that returns from SMM
    */
   control.conceal_vmx_from_pt = true;

   /*
    * true: IA32_RTIT_CTL MSR is loaded on VM entry
    */
   control.load_ia32_rtit_ctl = false;

   /*
    * true: UINV is loaded on VM entry
    */
   control.load_uinv = false;

   /*
    * true: CET-related MSRs and SPP are loaded on VM entry
    */
   control.load_cet_state = false;

   /*
    * true: guest IA32_LBR_CTL MSR is loaded on VM entry
    */
   control.load_guest_ia32_lbr_ctl = false;

   /*
    * true: PKRS is loaded on VM entry
    */
   control.load_pkrs = false;
   
   return control;
}

static void vmcs_setup_control (vcpu_ctx_t *vcpu_ctx)
{
   __vmx_pinbased_controls pinbased_ctl;
   __vmx_procbased_ctls procbased_ctl;
   __vmx_procbased_ctls2 procbased_ctl2;
   __vmx_exit_ctls exit_ctl;
   __vmx_entry_ctls entry_ctl;

   u8 true_controls = vcpu_ctx->cached.vmx_basic.fields.vmx_cap_support;

   // pinbased vm-execution controls
   pinbased_ctl = vmcs_setup_pinbased_ctls ();
   vmcs_adjust_controls (&pinbased_ctl.ctl, true_controls == 1
         ? IA32_VMX_PINBASED_CTLS_MSR
         : IA32_VMX_TRUE_PINBASED_CTLS_MSR);
   __vmx_vmwrite (VMCS_CTRL_PINBASED_CONTROLS, pinbased_ctl.ctl);

   // primary processor based vm-execution controls
   procbased_ctl = vmcs_setup_primary_procbased_ctls ();
   vmcs_adjust_controls (&procbased_ctl.ctl, true_controls == 1
         ? IA32_VMX_PROCBASED_CTLS_MSR
         : IA32_VMX_TRUE_PROCBASED_CTLS_MSR);
   __vmx_vmwrite (VMCS_CTRL_PROCBASED_CTLS, procbased_ctl.ctl);

   // secondary processor based vm-execution controls
   procbased_ctl2 = vmcs_setup_secondary_procbased_ctls ();
   vmcs_adjust_controls (&procbased_ctl2.ctl, IA32_VMX_PROCBASED_CTLS2_MSR);
   __vmx_vmwrite (VMCS_CTRL_PROCBASED_CTLS2, procbased_ctl2.ctl);

   // primary vm-exit controls
   exit_ctl = vmcs_setup_primary_exit_ctls ();
   vmcs_adjust_controls (&exit_ctl.ctl, true_controls == 1
         ? IA32_VMX_EXIT_CTLS_MSR
         : IA32_VMX_TRUE_EXIT_CTLS_MSR);
   __vmx_vmwrite (VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS, exit_ctl.ctl);

   // vm-entry controls
   entry_ctl = vmcs_setup_entry_ctls ();
   vmcs_adjust_controls (&entry_ctl.ctl, true_controls == 1
         ? IA32_VMX_ENTRY_CTLS_MSR
         : IA32_VMX_TRUE_ENTRY_CTLS_MSR);
   __vmx_vmwrite (VMCS_CTRL_VMENTRY_CONTROLS, entry_ctl.ctl);

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
