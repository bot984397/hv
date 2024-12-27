#include <linux/types.h>
#include <asm/msr.h>

#include "vmcs.h"
#include "asm.h"
#include "msr.h"
#include "enc.h"
#include "mem.h"
#include "vmx.h"
#include "crx.h"
#include "common.h"

static void vmcs_adjust_controls (u32 *ctl, u32 cap)
{
   ia32_generic_cap_msr cap_msr = {0};
   cap_msr.value = __rdmsr (cap);

   *ctl |= cap_msr.split.allowed_0;
   *ctl &= cap_msr.split.allowed_1;
}

static void vmcs_adjust_controls_ex (u32 *ctl, u32 cap1, u32 cap2, bool t)
{
   vmcs_adjust_controls (ctl, t == true ? cap2 : cap1);
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
    * true: EPT write permissions may be specified at 128 byte granularity
    */
   control.ept_sub_page_write_permissions = false;

   /*
    * true: all output addresses used by Intel PT are treated as guest
    *       physical addresses and translated using EPT
    */
   control.intel_pt_use_guest_phys_addr = false;

   /*
    * true: RDTSC / RDTSCP / RDMSR from IA32_TIME_STAMP_COUNTER MSR return a
    *       value modified by the TSC multiplier field
    */
   control.use_tsc_scaling = false;

   /*
    * false: executions of TPAUSE / UMONITOR / UMWAIT cause a #UD
    */
   control.enable_user_wait_and_pause = false;

   /*
    * false: executions of PCONFIG cause a #UD
    */
   control.enable_pconfig = false;

   /*
    * true: executions of ENCLV consult the ENCLV-exiting bitmap to determine
    *       wether the instruction causes a VM exit
    */
   control.enable_enclv_exiting = false;

   /*
    * true: assertion of a bus lock causes a VM exit
    */
   control.vmm_bus_lock_detection = false;

   /*
    * true: VM exit occurs if certain operations prevent the processor from
    *       reaching an instruction boundary within a specified amount of time
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

static __vmx_exception_bitmap vmcs_setup_exception_bitmap (void)
{
   __vmx_exception_bitmap control = {0};

   control.divide_error = false;

   control.debug = false;

   control.nmi_interrupt = false;

   control.breakpoint = false;

   control.overflow = false;

   control.bound_range_exceeded = false;

   control.invalid_opcode = false;

   control.device_not_available = false;

   control.double_fault = false;

   control.coprocessor_segment_overrun = false;

   control.invalid_tss = false;

   control.segment_not_present = false;

   control.stack_segment_fault = false;

   control.general_protection = false;

   control.page_fault = false;

   control.floating_point_error = false;

   control.alignment_check = false;

   control.machine_check = false;

   control.simd_floating_point_exception = false;

   control.virtualization_exception = false;

   control.control_protection_exception = false;

   return control;
}

/*
static void vmcs_set_msr_bitmap (vcpu_ctx_t *vcpu_ctx)
{
   mem_set_pages (vcpu_ctx->bitmaps.msr_bitmaps, 0xFF, 2);
}
*/

static void vmcs_set_msr_bitmap_single (u64 msr, bool w, vcpu_ctx_t *vcpu_ctx)
{
   switch (msr)
   {
      case 0x00000000 ... 0x00001FFF:
      {
         u64 byte = msr / 8;
         u64 bit = msr % 8;
         vcpu_ctx->bitmaps.msr_bitmaps[w == true ? 2048 + byte : byte] 
            |= (1 << bit);
         break;
      }
      case 0xC0000000 ... 0xC0001FFF:
      {
         u64 byte = (msr - 0xC0000000) / 8;
         u64 bit = (msr - 0xC0000000) % 8;
         vcpu_ctx->bitmaps.msr_bitmaps[w == true ? 3072 + byte : 1024 + byte]
            |= (1 << bit);
         break;
      }
      default:
         LOG_DBG ("MSR out of range");
   }
}

static void vmcs_clr_msr_bitmap (vcpu_ctx_t *vcpu_ctx)
{
   mem_zero_pages (vcpu_ctx->bitmaps.msr_bitmaps, 0);
}

/*
static void vmcs_clr_msr_bitmap_single (u64 msr, vcpu_ctx_t *vcpu_ctx)
{

}
*/

static void vmcs_setup_msr_bitmaps (vcpu_ctx_t *vcpu_ctx)
{
   vmcs_clr_msr_bitmap (vcpu_ctx);
   vmcs_set_msr_bitmap_single (0xC0001FFF, false, vcpu_ctx);
}

static void vmcs_setup_control (vcpu_ctx_t *vcpu_ctx)
{
   __vmx_pinbased_controls pinbased_ctl;
   __vmx_procbased_ctls procbased_ctl;
   __vmx_procbased_ctls2 procbased_ctl2;
   __vmx_exit_ctls exit_ctl;
   __vmx_entry_ctls entry_ctl;
   __vmx_exception_bitmap exception_bitmap;

   u8 true_controls = vcpu_ctx->cached.vmx_basic.vmx_cap_support;

   // pinbased vm-execution controls
   pinbased_ctl = vmcs_setup_pinbased_ctls ();
   vmcs_adjust_controls_ex (&pinbased_ctl.ctl, 
                            IA32_VMX_PINBASED_CTLS_MSR, 
                            IA32_VMX_TRUE_PINBASED_CTLS_MSR, 
                            true_controls);
   vmwrite (VMCS_CTRL_PINBASED_CONTROLS, pinbased_ctl.ctl);

   // primary processor based vm-execution controls
   procbased_ctl = vmcs_setup_primary_procbased_ctls ();
   vmcs_adjust_controls_ex (&procbased_ctl.ctl,
                            IA32_VMX_PROCBASED_CTLS_MSR,
                            IA32_VMX_TRUE_PROCBASED_CTLS_MSR,
                            true_controls);
   vmwrite (VMCS_CTRL_PROCBASED_CTLS, procbased_ctl.ctl);

   // secondary processor based vm-execution controls
   procbased_ctl2 = vmcs_setup_secondary_procbased_ctls ();
   vmcs_adjust_controls (&procbased_ctl2.ctl, IA32_VMX_PROCBASED_CTLS2_MSR);
   vmwrite (VMCS_CTRL_PROCBASED_CTLS2, procbased_ctl2.ctl);

   // primary vm-exit controls
   exit_ctl = vmcs_setup_primary_exit_ctls ();
   vmcs_adjust_controls_ex (&exit_ctl.ctl,
                            IA32_VMX_EXIT_CTLS_MSR,
                            IA32_VMX_TRUE_EXIT_CTLS_MSR,
                            true_controls);
   vmwrite (VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS, exit_ctl.ctl);

   // vm-entry controls
   entry_ctl = vmcs_setup_entry_ctls ();
   vmcs_adjust_controls_ex (&entry_ctl.ctl,
                            IA32_VMX_ENTRY_CTLS_MSR,
                            IA32_VMX_TRUE_ENTRY_CTLS_MSR,
                            true_controls);
   vmwrite (VMCS_CTRL_VMENTRY_CONTROLS, entry_ctl.ctl);

   // exception bitmap
   exception_bitmap = vmcs_setup_exception_bitmap ();
   vmwrite (VMCS_CTRL_EXCEPTION_BITMAP, exception_bitmap.ctl);

   // I/O bitmap addresses
   mem_zero_pages (vcpu_ctx->bitmaps.io_bitmap_a, 0);
   vmwrite (VMCS_CTRL_IO_BITMAP_A, vcpu_ctx->bitmaps.io_bitmap_a_phys);
   mem_zero_pages (vcpu_ctx->bitmaps.io_bitmap_b, 0);
   vmwrite (VMCS_CTRL_IO_BITMAP_B, vcpu_ctx->bitmaps.io_bitmap_b_phys);

   // Time-Stamp Counter offset and multiplier
   vmwrite (VMCS_CTRL_TSC_OFFSET, 0);
   vmwrite (VMCS_CTRL_TSC_MULTIPLIER, 0);

   // CR0 and CR4 shadowing / guest-host masks
   vmwrite (VMCS_CTRL_CR0_GUEST_HOST_MASK, 0);
   vmwrite (VMCS_CTRL_CR0_READ_SHADOW, __read_cr0 ());
   cr4_t cr4;
   cr4_t cr4_mask;
   cr4_mask.ctl = 0;
   cr4_mask.VMXE = 1;
   cr4.ctl = __read_cr4 ();
   cr4.VMXE = 0;
   vmwrite (VMCS_CTRL_CR4_GUEST_HOST_MASK, cr4_mask.ctl);
   vmwrite (VMCS_CTRL_CR4_READ_SHADOW, cr4.ctl);

   // CR3 target count and values
   vmwrite (VMCS_CTRL_CR3_TARGET_COUNT, 0);
   vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_0, 0);
   vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_1, 0);
   vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_2, 0);
   vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_3, 0);

   // APIC virtualization controls
   vmwrite (VMCS_CTRL_APIC_ACCESS_ADDRESS, 0);
   vmwrite (VMCS_CTRL_VIRTUAL_APIC_ADDRESS, 0);
   vmwrite (VMCS_CTRL_TPR_THRESHOLD, 0);
   vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_0, 0);
   vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_1, 0);
   vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_2, 0);
   vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_3, 0);
   vmwrite (VMCS_CTRL_POSTED_INTERRUPT_NOTIFICATION_VECTOR, 0);
   vmwrite (VMCS_CTRL_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS, 0);
   vmwrite (VMCS_CTRL_PID_POINTER_TABLE_ADDRESS, 0);
   vmwrite (VMCS_CTRL_LAST_PID_POINTER_INDEX, 0);

   // MSR bitmap address
   vmcs_setup_msr_bitmaps (vcpu_ctx);
   vmwrite (VMCS_CTRL_MSR_BITMAPS, vcpu_ctx->bitmaps.msr_bitmaps_phys);

   // Executive-VMCS pointer
   vmwrite (VMCS_CTRL_EXECUTIVE_VMCS_POINTER, 0);

   // Extended-Page-Table Pointer (EPTP)
   vmwrite (VMCS_CTRL_EPT_POINTER, 0);

   // Virtual-Processor Identifier (VPID)
   vmwrite (VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, 1);

   // Controls for PAUSE-Loop exiting
   vmwrite (VMCS_CTRL_PLE_GAP, 0);
   vmwrite (VMCS_CTRL_PLE_WINDOW, 0);

   // VM-Function Controls
   // TODO: implement vmcs_setup_vmfunc_ctls
   vmwrite (VMCS_CTRL_VM_FUNCTION_CONTROLS, 0);
   vmwrite (VMCS_CTRL_EPTP_LIST_ADDRESS, 0);

   // VMCS Shadowing Bitmap Addresses
   vmwrite (VMCS_CTRL_VMREAD_BITMAP_ADDRESS, 0);
   vmwrite (VMCS_CTRL_VMWRITE_BITMAP_ADDRESS, 0);

   // ENCLS-Exiting Bitmap
   vmwrite (VMCS_CTRL_ENCLS_EXITING_BITMAP, 0);

   // ENCLV-Exiting Bitmap
   vmwrite (VMCS_CTRL_ENCLV_EXITING_BITMAP, 0);

   // PCONFIG-Exiting Bitmap
   vmwrite (VMCS_CTRL_PCONFIG_EXITING_BITMAP, 0);

   // Control Field for Page-Modification logging
   vmwrite (VMCS_CTRL_PML_ADDRESS, 0);

   // Controls for Virtualization Exceptions
   vmwrite (VMCS_CTRL_VIRT_EXCEPTION_INFORMATION_ADDRESS, 0);
   vmwrite (VMCS_CTRL_EPTP_INDEX, 0);

   // XSS-Exiting Bitmap
   vmwrite (VMCS_CTRL_XSS_EXITING_BITMAP, 0);

   // Sub-Page-Permission-Table Pointer (SPPTP)
   vmwrite (VMCS_CTRL_SUB_PAGE_PERMISSION_TABLE_POINTER, 0);

   // HLAT related fields
   vmwrite (VMCS_CTRL_HLATP, 0);

   // PASID translation related fields
   vmwrite (VMCS_CTRL_LOW_PASID_DIRECTORY_ADDRESS, 0);
   vmwrite (VMCS_CTRL_HIGH_PASID_DIRECTORY_ADDRESS, 0);

   // Instruction-Timeout Control
   vmwrite (VMCS_CTRL_INSTRUCTION_TIMEOUT_CONTROL, 0);

   // IA32_SPEC_CTRL MSR virtualization controls
   vmwrite (VMCS_CTRL_IA32_SPEC_CTRL_MASK, 0);
   vmwrite (VMCS_CTRL_IA32_SPEC_CTRL_SHADOW, 0);

   // VM-Exit controls

   // VM-Exit controls for MSRs
   vmwrite (VMCS_CTRL_VMEXIT_MSR_STORE_COUNT, 0);
   vmwrite (VMCS_CTRL_VMEXIT_MSR_STORE_ADDRESS, 0);
   vmwrite (VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT, 0);
   vmwrite (VMCS_CTRL_VMEXIT_MSR_LOAD_ADDRESS, 0);

   // VM-Entry controls

   // VM-Entry controls for MSRs
   vmwrite (VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT, 0);
   vmwrite (VMCS_CTRL_VMENTRY_MSR_LOAD_ADDRESS, 0);

   // VM-Instruction error information field
   vmwrite (VMCS_RO_VM_INSTRUCTION_ERROR, 0);
}

static void vmcs_setup_host (vcpu_ctx_t *vcpu_ctx)
{
   // Control registers
   vmwrite (VMCS_HOST_CR0, __read_cr0 ());
   vmwrite (VMCS_HOST_CR3, __read_cr3 ());
   vmwrite (VMCS_HOST_CR4, __read_cr4 ());

   // RSP and RIP
   vmwrite (VMCS_HOST_RSP, 0);
   vmwrite (VMCS_HOST_RIP, 0);

   // Segment selectors

   // Segment base addresses

   // MSRs
   vmwrite (VMCS_HOST_IA32_SYSENTER_CS, __rdmsr (IA32_SYSENTER_CS_MSR));

   vmwrite (VMCS_HOST_IA32_SYSENTER_ESP, __rdmsr (IA32_SYSENTER_ESP_MSR));

   vmwrite (VMCS_HOST_IA32_SYSENTER_EIP, __rdmsr (IA32_SYSENTER_EIP_MSR));

   vmwrite (VMCS_HOST_IA32_PERF_GLOBAL_CTRL, __rdmsr (IA32_PERF_GLOBAL_CTRL_MSR));

   vmwrite (VMCS_HOST_IA32_PAT, __rdmsr (IA32_PAT_MSR));

   vmwrite (VMCS_HOST_IA32_EFER, __rdmsr (IA32_EFER_MSR));

   vmwrite (VMCS_HOST_IA32_S_CET, __rdmsr (IA32_S_CET_MSR));

   vmwrite (VMCS_HOST_IA32_ISSPT_ADDR, __rdmsr (IA32_ISSPT_ADDR_MSR));

   vmwrite (VMCS_HOST_IA32_PKRS, __rdmsr (IA32_PKRS_MSR));

   // Shadow-Stack Pointer (SSP) register
   vmwrite (VMCS_HOST_SSP, 0);
}

static void vmcs_setup_guest (vcpu_ctx_t *vcpu_ctx)
{
   // Control registers
   vmwrite (VMCS_GUEST_CR0, __read_cr0 ());
   vmwrite (VMCS_GUEST_CR3, __read_cr3 ());
   vmwrite (VMCS_GUEST_CR4, __read_cr4 ());

   // Debug register DR7
   vmwrite (VMCS_GUEST_DR7, __read_dr (7));

   // RSP, RIP and RFLAGS
   vmwrite (VMCS_GUEST_RSP, 0);
   vmwrite (VMCS_GUEST_RIP, 0);
   vmwrite (VMCS_GUEST_RFLAGS, read_rflags ());

   // Segment selectors
   vmwrite (VMCS_GUEST_CS_SELECTOR, 0);
   vmwrite (VMCS_GUEST_SS_SELECTOR, 0);
   vmwrite (VMCS_GUEST_DS_SELECTOR, 0);
   vmwrite (VMCS_GUEST_ES_SELECTOR, 0);
   vmwrite (VMCS_GUEST_FS_SELECTOR, 0);
   vmwrite (VMCS_GUEST_GS_SELECTOR, 0);
   vmwrite (VMCS_GUEST_TR_SELECTOR, 0);
   vmwrite (VMCS_GUEST_LDTR_SELECTOR, 0);

   // Segment base addresses - CS/SS/DS/ES fixed to 0 in long mode
   vmwrite (VMCS_GUEST_CS_BASE, 0);
   vmwrite (VMCS_GUEST_SS_BASE, 0);
   vmwrite (VMCS_GUEST_DS_BASE, 0);
   vmwrite (VMCS_GUEST_ES_BASE, 0);
   vmwrite (VMCS_GUEST_FS_BASE, __rdmsr (IA32_FS_BASE_MSR));
   vmwrite (VMCS_GUEST_GS_BASE, __rdmsr (IA32_GS_BASE_MSR));
   vmwrite (VMCS_GUEST_TR_BASE, 0);
   vmwrite (VMCS_GUEST_LDTR_BASE, 0);

   // Segment limits - CS/SS/DS/ES fixed to 0 in long mode
   vmwrite (VMCS_GUEST_CS_LIMIT, 0);
   vmwrite (VMCS_GUEST_SS_LIMIT, 0);
   vmwrite (VMCS_GUEST_DS_LIMIT, 0);
   vmwrite (VMCS_GUEST_ES_LIMIT, 0);
   vmwrite (VMCS_GUEST_FS_LIMIT, seglimit (read_fs ()));
   vmwrite (VMCS_GUEST_GS_LIMIT, seglimit (read_gs ()));
   vmwrite (VMCS_GUEST_TR_LIMIT, seglimit (read_tr ()));
   vmwrite (VMCS_GUEST_LDTR_LIMIT, seglimit (read_ldtr ()));

   // Access rights
   vmwrite (VMCS_GUEST_CS_ACCESS_RIGHTS, 0);
   vmwrite (VMCS_GUEST_SS_ACCESS_RIGHTS, 0);
   vmwrite (VMCS_GUEST_DS_ACCESS_RIGHTS, 0);
   vmwrite (VMCS_GUEST_ES_ACCESS_RIGHTS, 0);
   vmwrite (VMCS_GUEST_FS_ACCESS_RIGHTS, 0);
   vmwrite (VMCS_GUEST_GS_ACCESS_RIGHTS, 0);
   vmwrite (VMCS_GUEST_TR_ACCESS_RIGHTS, 0);
   vmwrite (VMCS_GUEST_LDTR_ACCESS_RIGHTS, 0);

   // MSRs
   vmwrite (VMCS_GUEST_IA32_DEBUGCTL, __rdmsr (IA32_DEBUGCTL_MSR));

   vmwrite (VMCS_GUEST_IA32_SYSENTER_CS, __rdmsr (IA32_SYSENTER_CS_MSR));

   vmwrite (VMCS_GUEST_IA32_SYSENTER_ESP, __rdmsr (IA32_SYSENTER_ESP_MSR));

   vmwrite (VMCS_GUEST_IA32_SYSENTER_EIP, __rdmsr (IA32_SYSENTER_EIP_MSR));

   vmwrite (VMCS_GUEST_IA32_PERF_GLOBAL_CTRL, __rdmsr (IA32_PERF_GLOBAL_CTRL_MSR));

   vmwrite (VMCS_GUEST_IA32_PAT, __rdmsr (IA32_PAT_MSR));

   vmwrite (VMCS_GUEST_IA32_EFER, __rdmsr (IA32_EFER_MSR));

   vmwrite (VMCS_GUEST_IA32_BNDCFGS, __rdmsr (IA32_BNDCFGS_MSR));

   vmwrite (VMCS_GUEST_IA32_RTIT_CTL, __rdmsr (IA32_RTIT_CTL_MSR));

   vmwrite (VMCS_GUEST_IA32_LBR_CTL, __rdmsr (IA32_LBR_CTL_MSR));

   vmwrite (VMCS_GUEST_IA32_S_CET, __rdmsr (IA32_S_CET_MSR));

   vmwrite (VMCS_GUEST_IA32_ISSPT_ADDR, __rdmsr (IA32_ISSPT_ADDR_MSR));

   vmwrite (VMCS_GUEST_IA32_PKRS, __rdmsr (IA32_PKRS_MSR));

   // Shadow-Stack Pointer (SSP) register
   vmwrite (VMCS_GUEST_SSP, 0);

   // SMBASE register
   vmwrite (VMCS_GUEST_SMBASE, 0);

   // Activity state
   vmwrite (VMCS_GUEST_ACTIVITY_STATE, 0);

   // Interruptibility state
   vmwrite (VMCS_GUEST_INTERRUPTIBILITY_STATE, 0);

   // Pending debug exceptions
   vmwrite (VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, 0);

   // VMCS link pointer
   vmwrite (VMCS_GUEST_VMCS_LINK_POINTER, U64_MAX);

   // VMX-preemption timer value
   vmwrite (VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE, 0);

   // Page-Directory-Pointer-Table entries (PDPTEs)
   vmwrite (VMCS_GUEST_PDPTE0, 0);
   vmwrite (VMCS_GUEST_PDPTE1, 0);
   vmwrite (VMCS_GUEST_PDPTE2, 0);
   vmwrite (VMCS_GUEST_PDPTE3, 0);

   // Guest interrupt status
   vmwrite (VMCS_GUEST_INTERRUPT_STATUS, 0);

   // PML index
   vmwrite (VMCS_GUEST_PML_INDEX, 0);
}

__attribute__((warn_unused_result)) 
int vmcs_setup (vcpu_ctx_t *vcpu_ctx)
{
   if (vmclear (vcpu_ctx->vmcs_physical) != 0)
   {
      LOG_DBG ("VMCLEAR: %s", vmx_get_error_message ());
      return 0;
   }
   if (vmptrld (vcpu_ctx->vmcs_physical) != 0)
   {
      LOG_DBG ("VMPTRLD: %s", vmx_get_error_message ());
      return 0;
   }

   vmcs_setup_control (vcpu_ctx);
   
   vmcs_setup_host (vcpu_ctx);

   vmcs_setup_guest (vcpu_ctx);

   if (VMX_ERR (vmlaunch ()))
   {
      LOG_DBG ("VMLAUNCH: %s", vmx_get_error_message ());
      return 0;
   }

   return 1;
}
