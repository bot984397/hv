#include <linux/types.h>
#include <asm/msr.h>

#include "vmcs.h"
#include "cpu.h"
#include "enc.h"
#include "mem.h"
#include "vmx.h"
#include "exit.h"
#include "intrin.h"
#include "common.h"

static bool vmcs_check (cpu_ctx *_cpu_ctx);

static void vmcs_adjust_controls (u32 *ctl, u64 cap)
{
   // low 32 bits = allowed 0 - high 32 bits = allowed 1
   *ctl |= (u32)(cap);
   *ctl &= (u32)(cap >> 32);
}

#define VMCS_ADJUST_CONTROLS1(cap, ctls, type) \
   cap = rdmsr1 (type); \
   vmcs_adjust_controls (ctls, cap);

#define VMCS_ADJUST_CONTROLS2(cap, ctls, type, cpu) \
   cap = rdmsr1 (cpu->cached.vmx_basic.vmx_cap_support == 1 \
      ? IA32_VMX_TRUE_##type##_CTLS \
      : IA32_VMX_##type##_CTLS); \
   vmcs_adjust_controls (ctls, cap);

static vmx_pinbased_ctls vmcs_setup_pinbased_ctls (void)
{
   vmx_pinbased_ctls control = {0};

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

static vmx_procbased_ctls vmcs_setup_procbased_ctls (void)
{
   vmx_procbased_ctls control = {0};

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

static vmx_procbased_ctls2 vmcs_setup_procbased_ctls2 (void)
{
   vmx_procbased_ctls2 control = {0};

   /*
    * true: accesses to page with APIC-access address are treated specially
    */
   control.virtualize_apic_accesses = false;

   /*
    * true: Extended Page Tables (EPT) are enabled
    */
   control.enable_ept = false;

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

static vmx_exit_ctls vmcs_setup_exit_ctls (void)
{
   vmx_exit_ctls control = {0};

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

static vmx_entry_ctls vmcs_setup_entry_ctls (void)
{
   vmx_entry_ctls control = {0};

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

static vmx_exception_bitmap vmcs_setup_exception_bitmap (void)
{
   vmx_exception_bitmap control = {0};

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
static void vmcs_set_msr_bitmap_single (u64 msr, bool w, cpu_ctx *_cpu_ctx)
{
   switch (msr)
   {
      case 0x00000000 ... 0x00001FFF:
      {
         u64 byte = msr / 8;
         _cpu_ctx->bitmaps.msr_bitmaps[w == true ? 2048 + byte : byte] 
            |= (1 << (msr % 8));
         break;
      }
      case 0xC0000000 ... 0xC0001FFF:
      {
         u64 byte = (msr - 0xC0000000) / 8;
         _cpu_ctx->bitmaps.msr_bitmaps[w == true ? 3072 + byte : 1024 + byte]
            |= (1 << ((msr - 0xC0000000) % 8));
         break;
      }
      default:
         LOG_DBG ("MSR out of range");
   }
}
*/

static void vmcs_clr_msr_bitmap (cpu_ctx *_cpu_ctx)
{
   page_zero (_cpu_ctx->bitmaps.msr_bitmaps, 0);
}

static void vmcs_setup_msr_bitmaps (cpu_ctx *_cpu_ctx)
{
   vmcs_clr_msr_bitmap (_cpu_ctx);
   //vmcs_set_msr_bitmap_single (0xC0001FFF, false, _cpu_ctx);
}

static u32 get_access_rights (u16 seg)
{
   segment_selector selector;
   segment_access_rights ar;

   selector.ctl = seg;

   if (selector.table_indicator == 0 && selector.index == 0)
   {
      ar.ctl = 0;
      ar.segment_unusable = 1;
      return ar.ctl;
   }

   ar.ctl = (lar (seg) >> 8);
   ar.segment_unusable = 0;
   ar.reserved_0 = 0;
   ar.reserved_1 = 0;

   return ar.ctl;
}

static u64 get_segment_base (u64 gdt_base, u16 seg)
{
   u64 seg_base;
   segment_selector selector;
   segment_descriptor_32 *descriptor;
   segment_descriptor_32 *descriptor_table;

   selector.ctl = seg;

   if (selector.table_indicator == 0 && selector.index == 0)
   {
      seg_base = 0;
      return seg_base;
   }

   descriptor_table = (segment_descriptor_32 *)gdt_base;
   descriptor = &descriptor_table[selector.index];

   seg_base = (u64)((descriptor->base_high & 0xFF000000) |
            ((descriptor->base_mid << 16) & 0x00FF0000) |
            ((descriptor->base_low >> 16) & 0x0000FFFF));

   if ((descriptor->available_for_system == 0) &&
         ((descriptor->descriptor_type == TSS_AVAILABLE) ||
         (descriptor->descriptor_type == TSS_BUSY)))
   {
      segment_descriptor_64 *expanded_descriptor;
      expanded_descriptor = (segment_descriptor_64 *)descriptor;
      seg_base |= ((u64)expanded_descriptor->base_upper << 32);
   }

   return seg_base;
}

static bool vmcs_init_control (cpu_ctx *_cpu_ctx)
{
   u64 e = 0, cap = 0;

   // pinbased vm-execution controls
   vmx_pinbased_ctls pinbased_ctl = vmcs_setup_pinbased_ctls ();
   VMCS_ADJUST_CONTROLS2 (cap, &pinbased_ctl.ctl, PINBASED, _cpu_ctx);
   e |= vmwrite (VMCS_CTRL_PINBASED_CTLS, pinbased_ctl.ctl);

   // primary processor based vm-execution controls
   vmx_procbased_ctls procbased_ctl = vmcs_setup_procbased_ctls ();
   VMCS_ADJUST_CONTROLS2 (cap, &procbased_ctl.ctl, PROCBASED, _cpu_ctx);
   e |= vmwrite (VMCS_CTRL_PROCBASED_CTLS, procbased_ctl.ctl);

   // secondary processor based vm-execution controls
   vmx_procbased_ctls2 procbased_ctl2 = vmcs_setup_procbased_ctls2 ();
   VMCS_ADJUST_CONTROLS1 (cap, &procbased_ctl2.ctl, IA32_VMX_PROCBASED_CTLS2);
   e |= vmwrite (VMCS_CTRL_PROCBASED_CTLS2, procbased_ctl2.ctl);

   // primary vm-exit controls
   vmx_exit_ctls exit_ctl = vmcs_setup_exit_ctls ();
   VMCS_ADJUST_CONTROLS2 (cap, &exit_ctl.ctl, EXIT, _cpu_ctx);
   e |= vmwrite (VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS, exit_ctl.ctl);

   // vm-entry controls
   vmx_entry_ctls entry_ctl = vmcs_setup_entry_ctls ();
   VMCS_ADJUST_CONTROLS2 (cap, &entry_ctl.ctl, ENTRY, _cpu_ctx);
   e |= vmwrite (VMCS_CTRL_VMENTRY_CONTROLS, entry_ctl.ctl);

   // exception bitmap
   vmx_exception_bitmap exception_bitmap = vmcs_setup_exception_bitmap ();
   e |= vmwrite (VMCS_CTRL_EXCEPTION_BITMAP, exception_bitmap.ctl);

   // I/O bitmap addresses
   page_zero (_cpu_ctx->bitmaps.io_bitmap_a, 0);
   e |= vmwrite (VMCS_CTRL_IO_BITMAP_A, _cpu_ctx->bitmaps.io_bitmap_a_phys);
   page_zero (_cpu_ctx->bitmaps.io_bitmap_b, 0);
   e |= vmwrite (VMCS_CTRL_IO_BITMAP_B, _cpu_ctx->bitmaps.io_bitmap_b_phys);

   // Page fault error code mask and match
   e |= vmwrite (VMCS_CTRL_PAGE_FAULT_ERROR_CODE_MASK, 0);
   e |= vmwrite (VMCS_CTRL_PAGE_FAULT_ERROR_CODE_MATCH, 0);

   // Time-Stamp Counter offset and multiplier
   e |= vmwrite (VMCS_CTRL_TSC_OFFSET, 0);
   e |= vmwrite (VMCS_CTRL_TSC_MULTIPLIER, 0);

   // CR0 and CR4 shadowing / guest-host masks
   e |= vmwrite (VMCS_CTRL_CR0_READ_SHADOW, readcr0 ());
   e |= vmwrite (VMCS_CTRL_CR4_READ_SHADOW, readcr4 ());
   e |= vmwrite (VMCS_CTRL_CR0_GUEST_HOST_MASK, 0);
   e |= vmwrite (VMCS_CTRL_CR4_GUEST_HOST_MASK, 0);

   // CR3 target count and values
   e |= vmwrite (VMCS_CTRL_CR3_TARGET_COUNT, 0);
   /*
   e |= vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_0, 0);
   e |= vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_1, 0);
   e |= vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_2, 0);
   e |= vmwrite (VMCS_CTRL_CR3_TARGET_VALUE_3, 0);
   */

   // APIC virtualization controls
   /*
   e |= vmwrite (VMCS_CTRL_APIC_ACCESS_ADDRESS, 0);
   e |= vmwrite (VMCS_CTRL_VIRTUAL_APIC_ADDRESS, 0);
   e |= vmwrite (VMCS_CTRL_TPR_THRESHOLD, 0);
   e |= vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_0, 0);
   e |= vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_1, 0);
   e |= vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_2, 0);
   e |= vmwrite (VMCS_CTRL_EOI_EXIT_BITMAP_3, 0);
   e |= vmwrite (VMCS_CTRL_POSTED_INTERRUPT_NOTIFICATION_VECTOR, 0);
   e |= vmwrite (VMCS_CTRL_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS, 0);
   e |= vmwrite (VMCS_CTRL_PID_POINTER_TABLE_ADDRESS, 0);
   e |= vmwrite (VMCS_CTRL_LAST_PID_POINTER_INDEX, 0);
   */

   // MSR bitmap address
   vmcs_setup_msr_bitmaps (_cpu_ctx);
   e |= vmwrite (VMCS_CTRL_MSR_BITMAPS, _cpu_ctx->bitmaps.msr_bitmaps_phys);

   /*
   // Executive-VMCS pointer
   e |= vmwrite (VMCS_CTRL_EXECUTIVE_VMCS_POINTER, 0);

   // Extended-Page-Table Pointer (EPTP)
   e |= vmwrite (VMCS_CTRL_EPT_POINTER, 0);
   */

   // Virtual-Processor Identifier (VPID)
   e |= vmwrite (VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, 1);

   /*
   // Controls for PAUSE-Loop exiting
   e |= vmwrite (VMCS_CTRL_PLE_GAP, 0);
   e |= vmwrite (VMCS_CTRL_PLE_WINDOW, 0);
   */

   /*
   // VM-Function Controls
   // todo: implement vmcs_setup_vmfunc_ctls
   e |= vmwrite (VMCS_CTRL_VM_FUNCTION_CONTROLS, 0);
   e |= vmwrite (VMCS_CTRL_EPTP_LIST_ADDRESS, 0);

   // VMCS Shadowing Bitmap Addresses
   e |= vmwrite (VMCS_CTRL_VMREAD_BITMAP_ADDRESS, 0);
   e |= vmwrite (VMCS_CTRL_VMWRITE_BITMAP_ADDRESS, 0);

   // ENCLS-Exiting Bitmap
   e |= vmwrite (VMCS_CTRL_ENCLS_EXITING_BITMAP, 0);

   // ENCLV-Exiting Bitmap
   e |= vmwrite (VMCS_CTRL_ENCLV_EXITING_BITMAP, 0);

   // PCONFIG-Exiting Bitmap
   e |= vmwrite (VMCS_CTRL_PCONFIG_EXITING_BITMAP, 0);

   // Control Field for Page-Modification logging
   e |= vmwrite (VMCS_CTRL_PML_ADDRESS, 0);
   */

   /*
   // Controls for Virtualization Exceptions
   e |= vmwrite (VMCS_CTRL_VIRT_EXCEPTION_INFORMATION_ADDRESS, 0);
   e |= vmwrite (VMCS_CTRL_EPTP_INDEX, 0);

   // XSS-Exiting Bitmap
   e |= vmwrite (VMCS_CTRL_XSS_EXITING_BITMAP, 0);
   */

   /*
   // Sub-Page-Permission-Table Pointer (SPPTP)
   e |= vmwrite (VMCS_CTRL_SUB_PAGE_PERMISSION_TABLE_POINTER, 0);

   // HLAT related fields
   e |= vmwrite (VMCS_CTRL_HLATP, 0);

   // PASID translation related fields
   e |= vmwrite (VMCS_CTRL_LOW_PASID_DIRECTORY_ADDRESS, 0);
   e |= vmwrite (VMCS_CTRL_HIGH_PASID_DIRECTORY_ADDRESS, 0);

   // Instruction-Timeout Control
   e |= vmwrite (VMCS_CTRL_INSTRUCTION_TIMEOUT_CONTROL, 0);

   // IA32_SPEC_CTRL MSR virtualization controls
   e |= vmwrite (VMCS_CTRL_IA32_SPEC_CTRL_MASK, 0);
   e |= vmwrite (VMCS_CTRL_IA32_SPEC_CTRL_SHADOW, 0);
   */

   // VM-Exit controls

   // VM-Exit controls for MSRs
   e |= vmwrite (VMCS_CTRL_VMEXIT_MSR_STORE_COUNT, 0);
   /*
   e |= vmwrite (VMCS_CTRL_VMEXIT_MSR_STORE_ADDRESS, 0);
   */
   e |= vmwrite (VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT, 0);
   /*
   e |= vmwrite (VMCS_CTRL_VMEXIT_MSR_LOAD_ADDRESS, 0);
   */

   // VM-Entry controls

   // VM-Entry controls for MSRs
   e |= vmwrite (VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT, 0);
   /*
   e |= vmwrite (VMCS_CTRL_VMENTRY_MSR_LOAD_ADDRESS, 0);
   */

   // VM-Instruction error information field
   e |= vmwrite (VMCS_RO_VM_INSTRUCTION_ERROR, 0);

   return e == 0;
}

static bool vmcs_init_host (cpu_ctx *_cpu_ctx)
{
   u64 e = 0;

   // Control registers
   e |= vmwrite (VMCS_HOST_CR0, readcr0 ());
   e |= vmwrite (VMCS_HOST_CR3, readcr3 ());
   e |= vmwrite (VMCS_HOST_CR4, readcr4 ());

   // RSP and RIP
   e |= vmwrite (VMCS_HOST_RSP, (u64)&_cpu_ctx->_vmm_stk->_cpu_ctx);
   e |= vmwrite (VMCS_HOST_RIP, (u64)&vmexit_start);

   // Segment selectors
   e |= vmwrite (VMCS_HOST_CS_SELECTOR, read_cs () & VMCS_HOST_SELECTOR_MASK);
   e |= vmwrite (VMCS_HOST_SS_SELECTOR, read_ss () & VMCS_HOST_SELECTOR_MASK);
   e |= vmwrite (VMCS_HOST_DS_SELECTOR, read_ds () & VMCS_HOST_SELECTOR_MASK);
   e |= vmwrite (VMCS_HOST_ES_SELECTOR, read_es () & VMCS_HOST_SELECTOR_MASK);
   e |= vmwrite (VMCS_HOST_FS_SELECTOR, read_fs () & VMCS_HOST_SELECTOR_MASK);
   e |= vmwrite (VMCS_HOST_GS_SELECTOR, read_gs () & VMCS_HOST_SELECTOR_MASK);
   e |= vmwrite (VMCS_HOST_TR_SELECTOR, read_tr () & VMCS_HOST_SELECTOR_MASK);

   // Segment base addresses
   pseudo_descriptor gdt = sgdt ();
   pseudo_descriptor idt = sidt ();

   e |= vmwrite (VMCS_HOST_FS_BASE, __rdmsr (IA32_FS_BASE));
   e |= vmwrite (VMCS_HOST_GS_BASE, __rdmsr (IA32_GS_BASE));
   e |= vmwrite (VMCS_HOST_TR_BASE, get_segment_base (gdt.base, read_tr ()));
   e |= vmwrite (VMCS_HOST_GDTR_BASE, gdt.base);
   e |= vmwrite (VMCS_HOST_IDTR_BASE, idt.base);

   // MSRs
   e |= vmwrite (VMCS_HOST_IA32_SYSENTER_CS, __rdmsr (IA32_SYSENTER_CS));
   e |= vmwrite (VMCS_HOST_IA32_SYSENTER_ESP, __rdmsr (IA32_SYSENTER_ESP));
   e |= vmwrite (VMCS_HOST_IA32_SYSENTER_EIP, __rdmsr (IA32_SYSENTER_EIP));
   e |= vmwrite (VMCS_HOST_IA32_PERF_GLOBAL_CTRL, __rdmsr (IA32_PERF_GLOBAL_CTRL));
   e |= vmwrite (VMCS_HOST_IA32_PAT, __rdmsr (IA32_PAT));
   e |= vmwrite (VMCS_HOST_IA32_EFER, __rdmsr (IA32_EFER));
   /*
   e |= vmwrite (VMCS_HOST_IA32_S_CET, __rdmsr (IA32_S_CET_MSR));
   e |= vmwrite (VMCS_HOST_IA32_ISSPT_ADDR, __rdmsr (IA32_ISSPT_ADDR_MSR));
   e |= vmwrite (VMCS_HOST_IA32_PKRS, __rdmsr (IA32_PKRS_MSR));

   // Shadow-Stack Pointer (SSP) register
   e |= vmwrite (VMCS_HOST_SSP, 0);
   */

   return e == 0;
}

static bool vmcs_init_guest (cpu_ctx *_cpu_ctx, guest_init guest)
{
   u64 e = 0;

   // Control registers
   e |= vmwrite (VMCS_GUEST_CR0, readcr0 ());
   e |= vmwrite (VMCS_GUEST_CR3, readcr3 ());
   e |= vmwrite (VMCS_GUEST_CR4, readcr4 ());

   // Debug register DR7
   e |= vmwrite (VMCS_GUEST_DR7, __read_dr (7));

   // RSP, RIP and RFLAGS
   e |= vmwrite (VMCS_GUEST_RSP, guest.sp);
   e |= vmwrite (VMCS_GUEST_RIP, guest.ip);
   e |= vmwrite (VMCS_GUEST_RFLAGS, guest.rf);

   // Segment selectors
   e |= vmwrite (VMCS_GUEST_CS_SELECTOR, read_cs ());
   e |= vmwrite (VMCS_GUEST_SS_SELECTOR, read_ss ());
   e |= vmwrite (VMCS_GUEST_DS_SELECTOR, read_ds ());
   e |= vmwrite (VMCS_GUEST_ES_SELECTOR, read_es ());
   e |= vmwrite (VMCS_GUEST_FS_SELECTOR, read_fs ());
   e |= vmwrite (VMCS_GUEST_GS_SELECTOR, read_gs ());
   e |= vmwrite (VMCS_GUEST_TR_SELECTOR, read_tr ());
   e |= vmwrite (VMCS_GUEST_LDTR_SELECTOR, 0);

   pseudo_descriptor gdt = sgdt ();
   pseudo_descriptor idt = sidt ();

   // Segment base addresses - CS/SS/DS/ES fixed to 0 in long mode
   e |= vmwrite (VMCS_GUEST_CS_BASE, get_segment_base (gdt.base, read_cs ()));
   e |= vmwrite (VMCS_GUEST_SS_BASE, get_segment_base (gdt.base, read_ss ()));
   e |= vmwrite (VMCS_GUEST_DS_BASE, get_segment_base (gdt.base, read_ds ()));
   e |= vmwrite (VMCS_GUEST_ES_BASE, get_segment_base (gdt.base, read_es ()));
   e |= vmwrite (VMCS_GUEST_FS_BASE, __rdmsr (IA32_FS_BASE));
   e |= vmwrite (VMCS_GUEST_GS_BASE, __rdmsr (IA32_GS_BASE));
   e |= vmwrite (VMCS_GUEST_TR_BASE, get_segment_base (gdt.base, read_tr ()));
   e |= vmwrite (VMCS_GUEST_LDTR_BASE, get_segment_base (gdt.base, read_ldtr ()));

   // Segment limits - CS/SS/DS/ES fixed to 0 in long mode
   e |= vmwrite (VMCS_GUEST_CS_LIMIT, seglimit (read_cs ()));
   e |= vmwrite (VMCS_GUEST_SS_LIMIT, seglimit (read_ss ()));
   e |= vmwrite (VMCS_GUEST_DS_LIMIT, seglimit (read_ds ()));
   e |= vmwrite (VMCS_GUEST_ES_LIMIT, seglimit (read_es ()));
   e |= vmwrite (VMCS_GUEST_FS_LIMIT, seglimit (read_fs ()));
   e |= vmwrite (VMCS_GUEST_GS_LIMIT, seglimit (read_gs ()));
   e |= vmwrite (VMCS_GUEST_TR_LIMIT, seglimit (read_tr ()));
   e |= vmwrite (VMCS_GUEST_LDTR_LIMIT, seglimit (read_ldtr ()));

   // Access rights
   e |= vmwrite (VMCS_GUEST_CS_ACCESS_RIGHTS, get_access_rights (read_cs ()));
   e |= vmwrite (VMCS_GUEST_SS_ACCESS_RIGHTS, get_access_rights (read_ss ()));
   e |= vmwrite (VMCS_GUEST_DS_ACCESS_RIGHTS, get_access_rights (read_ds ()));
   e |= vmwrite (VMCS_GUEST_ES_ACCESS_RIGHTS, get_access_rights (read_es ()));
   e |= vmwrite (VMCS_GUEST_FS_ACCESS_RIGHTS, get_access_rights (read_fs ()));
   e |= vmwrite (VMCS_GUEST_GS_ACCESS_RIGHTS, get_access_rights (read_gs ()));
   e |= vmwrite (VMCS_GUEST_TR_ACCESS_RIGHTS, get_access_rights (read_tr ()));
   e |= vmwrite (VMCS_GUEST_LDTR_ACCESS_RIGHTS, get_access_rights (read_ldtr ()));

   // GDTR and IDTR
   e |= vmwrite (VMCS_GUEST_GDTR_LIMIT, gdt.limit);
   e |= vmwrite (VMCS_GUEST_GDTR_BASE, gdt.base);
   e |= vmwrite (VMCS_GUEST_IDTR_LIMIT, idt.limit);
   e |= vmwrite (VMCS_GUEST_IDTR_BASE, idt.base);

   // MSRs
   e |= vmwrite (VMCS_GUEST_IA32_DEBUGCTL, __rdmsr (IA32_DEBUGCTL));
   e |= vmwrite (VMCS_GUEST_IA32_SYSENTER_CS, __rdmsr (IA32_SYSENTER_CS));
   e |= vmwrite (VMCS_GUEST_IA32_SYSENTER_ESP, __rdmsr (IA32_SYSENTER_ESP));
   e |= vmwrite (VMCS_GUEST_IA32_SYSENTER_EIP, __rdmsr (IA32_SYSENTER_EIP));
   e |= vmwrite (VMCS_GUEST_IA32_PERF_GLOBAL_CTRL, __rdmsr (IA32_PERF_GLOBAL_CTRL));
   e |= vmwrite (VMCS_GUEST_IA32_PAT, __rdmsr (IA32_PAT));
   e |= vmwrite (VMCS_GUEST_IA32_EFER, __rdmsr (IA32_EFER));
   e |= vmwrite (VMCS_GUEST_IA32_BNDCFGS, __rdmsr (IA32_BNDCFGS));
   /*
   e |= vmwrite (VMCS_GUEST_IA32_RTIT_CTL, __rdmsr (IA32_RTIT_CTL_MSR));
   e |= vmwrite (VMCS_GUEST_IA32_LBR_CTL, __rdmsr (IA32_LBR_CTL_MSR));
   e |= vmwrite (VMCS_GUEST_IA32_S_CET, __rdmsr (IA32_S_CET_MSR));
   e |= vmwrite (VMCS_GUEST_IA32_ISSPT_ADDR, __rdmsr (IA32_ISSPT_ADDR_MSR));
   e |= vmwrite (VMCS_GUEST_IA32_PKRS, __rdmsr (IA32_PKRS_MSR));
   */

   // Shadow-Stack Pointer (SSP) register
   //e |= vmwrite (VMCS_GUEST_SSP, 0);

   // SMBASE register
   /*
   e |= vmwrite (VMCS_GUEST_SMBASE, 0);
   */

   // Activity state
   e |= vmwrite (VMCS_GUEST_ACTIVITY_STATE, 0);

   // Interruptibility state
   e |= vmwrite (VMCS_GUEST_INTERRUPTIBILITY_STATE, 0);

   // Pending debug exceptions
   e |= vmwrite (VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, 0);

   // VMCS link pointer
   e |= vmwrite (VMCS_GUEST_VMCS_LINK_POINTER, ~0ULL);

   // VMX-preemption timer value
   e |= vmwrite (VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE, 0);

   // Page-Directory-Pointer-Table entries (PDPTEs)
   /*
   e |= vmwrite (VMCS_GUEST_PDPTE0, 0);
   e |= vmwrite (VMCS_GUEST_PDPTE1, 0);
   e |= vmwrite (VMCS_GUEST_PDPTE2, 0);
   e |= vmwrite (VMCS_GUEST_PDPTE3, 0);
   */

   // Guest interrupt status
   /*
   e |= vmwrite (VMCS_GUEST_INTERRUPT_STATUS, 0);
   */

   // PML index
   /*
   e |= vmwrite (VMCS_GUEST_PML_INDEX, 0);
   */

   return e == 0;
}

_warn_unused_result_ int vmcs_init_all (cpu_ctx *_cpu_ctx, guest_init guest)
{
   //u64 e;

   if (vmclear (_cpu_ctx->vmcs_physical) != 0)
   {
      return -EVMCLEAR;
   }
   if (vmptrld (_cpu_ctx->vmcs_physical) != 0)
   {
      return -EVMPTRLD;
   }

   if (!vmcs_init_control (_cpu_ctx))
      LOG_DBG ("control setup failed");
   if (!vmcs_init_host (_cpu_ctx))
      LOG_DBG ("host setup failed");
   if (!vmcs_init_guest (_cpu_ctx, guest))
      LOG_DBG ("guest setup failed");

   /*
   e |= vmcs_init_control (_cpu_ctx);
   e |= vmcs_init_host (_cpu_ctx);
   e |= vmcs_init_guest (_cpu_ctx, guest);
   if (e)
   {
      return -EVMSTATE;
   }
   */

   if (!vmcs_check (_cpu_ctx))
   {
      LOG_DBG ("failure");
      //return -EVMSTATE;
   }

   if (VMX_ERR (vmlaunch ()))
   {
      LOG_DBG ("VMLAUNCH: %s", vmx_get_error_message ());
      return 0;
   }

   return 1;
}

static bool vmcs_check_control_32 (u32 ctl, cap_msr cap)
{
   if ((ctl & ~cap._1) != 0 || (ctl & cap._0) != cap._0) {
      return false;
   }
   return true;
}

static bool vmcs_check (cpu_ctx *_cpu_ctx)
{
   // checks on vmx controls [28.2.1]
   cap_msr cap = {0};
   u64 ctl = 0;

   ctl = vmread1 (VMCS_CTRL_PINBASED_CTLS);
   cap.ctl = rdmsr1 (IA32_VMX_TRUE_PINBASED_CTLS);
   if (!vmcs_check_control_32 (ctl, cap))
   {
      return false;
   }

   ctl = vmread1 (VMCS_CTRL_PROCBASED_CTLS);
   cap.ctl = __rdmsr (IA32_VMX_TRUE_PROCBASED_CTLS);
   if (!vmcs_check_control_32 (ctl, cap))
   {
      //return false;
   }

   if (vmread1 (VMCS_CTRL_CR3_TARGET_COUNT) > 4)
   {
      VCPU_DBG ("invalid CR3 target count");
      return false;
   }

   if (_cpu_ctx->bitmaps.io_bitmap_a_phys & 0xFFF)
   {
      VCPU_DBG ("io bitmap a misaligned");
      return false;
   }

   if (_cpu_ctx->bitmaps.io_bitmap_b_phys & 0xFFF)
   {
      VCPU_DBG ("io bitmap b misaligned");
      return false;
   }

   if (_cpu_ctx->bitmaps.msr_bitmaps_phys & 0xFFF)
   {
      VCPU_DBG ("msr bitmaps misaligned");
   }

   /*
   if (procbased.use_tpr_shadow == 1)
   {
      if (vmread1 (VMCS_CTRL_VIRTUAL_APIC_ADDRESS) & 0xFFF)
      {
         VCPU_DBG ("virtual apic address misaligned");
         return false;
      }
   }
   */

   VCPU_DBG ("VMCS region OK");
   return true;
}
