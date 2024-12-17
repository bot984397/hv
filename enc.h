#ifndef __LKM_ENC_H__
#define __LKM_ENC_H__

typedef enum
{
   full = 0,
   high
} vmcs_access_t;

typedef enum
{
   control = 0,
   vm_exit,
   guest_state,
   host_state
} vmcs_type_t;

typedef enum
{
   word = 0,
   qword,
   dword,
   natural
} vmcs_width_t;

#define ENC_VMCS_COMPONENT(access, index, type, width) \
   (unsigned)((unsigned short)(access) | \
   ((unsigned short)(index) << 1) | \
   ((unsigned short)(type) << 10) | \
   ((unsigned short)(width) << 13))
#define ENC_VMCS_COMPONENT_FULL(index, type, width) \
   ENC_VMCS_COMPONENT(full, index, type, width)
#define ENC_VMCS_COMPONENT_HIGH(index, type, width) \
   ENC_VMCS_COMPONENT(high, index, type, width)
#define ENC_VMCS_COMPONENT_FULL_16(index, type) \
   ENC_VMCS_COMPONENT_FULL(index, type, word)
#define ENC_VMCS_COMPONENT_FULL_32(index, type) \
   ENC_VMCS_COMPONENT_FULL(index, type, dword)
#define ENC_VMCS_COMPONENT_FULL_64(index, type) \
   ENC_VMCS_COMPONENT_FULL(index, type, qword)
#define ENC_VMCS_COMPONENT_FULL_NAT(index, type) \
   ENC_VMCS_COMPONENT_FULL(index, type, natural)
#define ENC_VMCS_COMPONENT_HIGH_16(index, type) \
   ENC_VMCS_COMPONENT_HIGH(index, type, word)
#define ENC_VMCS_COMPONENT_HIGH_32(index, type) \
   ENC_VMCS_COMPONENT_HIGH(index, type, dword)
#define ENC_VMCS_COMPONENT_HIGH_64(index, type) \
   ENC_VMCS_COMPONENT_HIGH(index, type, qword)
#define ENC_VMCS_COMPONENT_HIGH_NAT(index, type) \
   ENC_VMCS_COMPONENT_HIGH(index, type, natural)

// 16-bit fields (control)
#define VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER \
   ENC_VMCS_COMPONENT_FULL_16 (0, control)
#define VMCS_CTRL_POSTED_INTERRUPT_NOTIFICATION_VECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (1, control)
#define VMCS_CTRL_EPTP_INDEX \
   ENC_VMCS_COMPONENT_FULL_16 (2, control)
#define VMCS_CTRL_HLAT_PREFIX_SIZE \
   ENC_VMCS_COMPONENT_FULL_16 (3, control)
#define VMCS_CTRL_LAST_PID_POINTER_INDEX \
   ENC_VMCS_COMPONENT_FULL_16 (4, control)

// 16-bit fields (guest-state)
#define VMCS_GUEST_ES_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (0, guest_state)
#define VMCS_GUEST_CS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (1, guest_state)
#define VMCS_GUEST_SS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (2, guest_state)
#define VMCS_GUEST_DS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (3, guest_state)
#define VMCS_GUEST_FS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (4, guest_state)
#define VMCS_GUEST_GS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (5, guest_state)
#define VMCS_GUEST_LDTR_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (6, guest_state)
#define VMCS_GUEST_TR_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (7, guest_state)
#define VMCS_GUEST_INTERRUPT_STATUS \
   ENC_VMCS_COMPONENT_FULL_16 (8, guest_state)
#define VMCS_GUEST_PML_INDEX \
   ENC_VMCS_COMPONENT_FULL_16 (9, guest_state)
#define VMCS_GUEST_UINV \
   ENC_VMCS_COMPONENT_FULL_16 (10, guest_state)

// 16-bit fields (host-state)
#define VMCS_HOST_ES_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (0, host_state)
#define VMCS_HOST_CS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (1, host_state)
#define VMCS_HOST_SS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (2, host_state)
#define VMCS_HOST_DS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (3, host_state)
#define VMCS_HOST_FS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (4, host_state)
#define VMCS_HOST_GS_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (5, host_state)
#define VMCS_HOST_TR_SELECTOR \
   ENC_VMCS_COMPONENT_FULL_16 (6, host_state)

// 64-bit fields (control)
#define VMCS_CTRL_IO_BITMAP_A_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (0, control)
#define VMCS_CTRL_IO_BITMAP_B_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (1, control)
#define VMCS_CTRL_MSR_BITMAP_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (2, control)
#define VMCS_CTRL_VMEXIT_MSR_STORE_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (3, control)
#define VMCS_CTRL_VMEXIT_MSR_LOAD_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (4, control)
#define VMCS_CTRL_VMENTRY_MSR_LOAD_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (5, control)
#define VMCS_CTRL_EXECUTIVE_VMCS_POINTER \
   ENC_VMCS_COMPONENT_FULL_64 (6, control)
#define VMCS_CTRL_PML_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (7, control)
#define VMCS_CTRL_TSC_OFFSET \
   ENC_VMCS_COMPONENT_FULL_64 (8, control)
#define VMCS_CTRL_VIRTUAL_APIC_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (9, control)
#define VMCS_CTRL_APIC_ACCESS_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (10, control)
#define VMCS_CTRL_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (11, control)
#define VMCS_CTRL_VM_FUNCTION_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_64 (12, control)
#define VMCS_CTRL_EPT_POINTER \
   ENC_VMCS_COMPONENT_FULL_64 (13, control)
#define VMCS_CTRL_EOI_EXIT_BITMAP_0 \
   ENC_VMCS_COMPONENT_FULL_64 (14, control)
#define VMCS_CTRL_EOI_EXIT_BITMAP_1 \
   ENC_VMCS_COMPONENT_FULL_64 (15, control)
#define VMCS_CTRL_EOI_EXIT_BITMAP_2 \
   ENC_VMCS_COMPONENT_FULL_64 (16, control)
#define VMCS_CTRL_EOI_EXIT_BITMAP_3 \
   ENC_VMCS_COMPONENT_FULL_64 (17, control)
#define VMCS_CTRL_EPTP_LIST_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (18, control)
#define VMCS_CTRL_VMREAD_BITMAP_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (19, control)
#define VMCS_CTRL_VMWRITE_BITMAP_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (20, control)
#define VMCS_CTRL_VIRT_EXCEPTION_INFORMATION_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (21, control)
#define VMCS_CTRL_XSS_EXITING_BITMAP \
   ENC_VMCS_COMPONENT_FULL_64 (22, control)
#define VMCS_CTRL_ENCLS_EXITING_BITMAP \
   ENC_VMCS_COMPONENT_FULL_64 (23, control)
#define VMCS_CTRL_SUB_PAGE_PERMISSION_TABLE_POINTER \
   ENC_VMCS_COMPONENT_FULL_64 (24, control)
#define VMCS_CTRL_TSC_MULTIPLIER \
   ENC_VMCS_COMPONENT_FULL_64 (25, control)
#define VMCS_CTRL_TERTIARY_PROCBASED_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_64 (26, control)
#define VMCS_CTRL_ENCLV_EXITING_BITMAP \
   ENC_VMCS_COMPONENT_FULL_64 (27, control)
#define VMCS_CTRL_LOW_PASID_DIRECTORY_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (28, control)
#define VMCS_CTRL_HIGH_PASID_DIRECTORY_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (29, control)
#define VMCS_CTRL_SHARED_EPT_POINTER \
   ENC_VMCS_COMPONENT_FULL_64 (30, control)
#define VMCS_CTRL_PCONFIG_EXITING_BITMAP \
   ENC_VMCS_COMPONENT_FULL_64 (31, control)
#define VMCS_CTRL_HLATP \
   ENC_VMCS_COMPONENT_FULL_64 (32, control)
#define VMCS_CTRL_PID_POINTER_TABLE_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (33, control)
#define VMCS_CTRL_SECONDARY_VMEXIT_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_64 (34, control)
#define VMCS_CTRL_IA32_SPEC_CTRL_MASK \
   ENC_VMCS_COMPONENT_FULL_64 (35, control)
#define VMCS_CTRL_IA32_SPEC_CTRL_SHADOW \
   ENC_VMCS_COMPONENT_FULL_64 (36, control)

// 64-bit fields (read-only data)
#define VMCS_RO_GUEST_PHYSICAL_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_64 (0, vm_exit)

// 64-bit fields (guest-state)
#define VMCS_GUEST_VMCS_LINK_POINTER \
   ENC_VMCS_COMPONENT_FULL_64 (0, guest_state)
#define VMCS_GUEST_IA32_DEBUGCTL \
   ENC_VMCS_COMPONENT_FULL_64 (1, guest_state)
#define VMCS_GUEST_IA32_PAT \
   ENC_VMCS_COMPONENT_FULL_64 (2, guest_state)
#define VMCS_GUEST_IA32_EFER \
   ENC_VMCS_COMPONENT_FULL_64 (3, guest_state)
#define VMCS_GUEST_IA32_PERF_GLOBAL_CTRL \
   ENC_VMCS_COMPONENT_FULL_64 (4, guest_state)
#define VMCS_GUEST_PDPTE0 \
   ENC_VMCS_COMPONENT_FULL_64 (5, guest_state)
#define VMCS_GUEST_PDPTE1 \
   ENC_VMCS_COMPONENT_FULL_64 (6, guest_state)
#define VMCS_GUEST_PDPTE2 \
   ENC_VMCS_COMPONENT_FULL_64 (7, guest_state)
#define VMCS_GUEST_PDPTE3 \
   ENC_VMCS_COMPONENT_FULL_64 (8, guest_state)
#define VMCS_GUEST_IA32_BNDCFGS \
   ENC_VMCS_COMPONENT_FULL_64 (9, guest_state)
#define VMCS_GUEST_IA32_RTIT_CTL \
   ENC_VMCS_COMPONENT_FULL_64 (10, guest_state)
#define VMCS_GUEST_IA32_LBR_CTL \
   ENC_VMCS_COMPONENT_FULL_64 (11, guest_state)
#define VMCS_GUEST_IA32_PKRS \
   ENC_VMCS_COMPONENT_FULL_64 (12, guest_state)

// 64-bit fields (host-state)
#define VMCS_HOST_IA32_PAT \
   ENC_VMCS_COMPONENT_FULL_64 (0, host_state)
#define VMCS_HOST_IA32_EFER \
   ENC_VMCS_COMPONENT_FULL_64 (1, host_state)
#define VMCS_HOST_IA32_PERF_GLOBAL_CTRL \
   ENC_VMCS_COMPONENT_FULL_64 (2, host_state)
#define VMCS_HOST_IA32_PKRS \
   ENC_VMCS_COMPONENT_FULL_64 (3, host_state)

// 32-bit fields (control)
#define VMCS_CTRL_PINBASED_EXECUTION_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_32 (0, control)
#define VMCS_CTRL_PRIMARY_PROCBASED_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_32 (1, control)
#define VMCS_CTRL_EXCEPTION_BITMAP \
   ENC_VMCS_COMPONENT_FULL_32 (2, control)
#define VMCS_CTRL_PAGE_FAULT_ERROR_CODE_MASK \
   ENC_VMCS_COMPONENT_FULL_32 (3, control)
#define VMCS_CTRL_PAGE_FAULT_ERROR_CODE_MATCH \
   ENC_VMCS_COMPONENT_FULL_32 (4, control)
#define VMCS_CTRL_CR3_TARGET_COUNT \
   ENC_VMCS_COMPONENT_FULL_32 (5, control)
#define VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_32 (6, control)
#define VMCS_CTRL_VMEXIT_MSR_STORE_COUNT \
   ENC_VMCS_COMPONENT_FULL_32 (7, control)
#define VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT \
   ENC_VMCS_COMPONENT_FULL_32 (8, control)
#define VMCS_CTRL_VMENTRY_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_32 (9, control)
#define VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT \
   ENC_VMCS_COMPONENT_FULL_32 (10, control)
#define VMCS_CTRL_VMENTRY_INTERRUPT_ERROR \
   ENC_VMCS_COMPONENT_FULL_32 (11, control)
#define VMCS_CTRL_VMENTRY_EXCEPT_ERROR_CODE \
   ENC_VMCS_COMPONENT_FULL_32 (12, control)
#define VMCS_CTRL_VMENTRY_INSTRUCTION_LENGTH \
   ENC_VMCS_COMPONENT_FULL_32 (13, control)
#define VMCS_CTRL_TPR_THRESHOLD \
   ENC_VMCS_COMPONENT_FULL_32 (14, control)
#define VMCS_CTRL_SECONDARY_PROCBASED_CONTROLS \
   ENC_VMCS_COMPONENT_FULL_32 (15, control)
#define VMCS_CTRL_PLE_GAP \
   ENC_VMCS_COMPONENT_FULL_32 (16, control)
#define VMCS_CTRL_PLE_WINDOW \
   ENC_VMCS_COMPONENT_FULL_32 (17, control)
#define VMCS_CTRL_INSTRUCTION_TIMEOUT_CONTROL \
   ENC_VMCS_COMPONENT_FULL_32 (18, control)

// 32-bit fields (read-only data)
#define VMCS_RO_VM_INSTRUCTION_ERROR \
   ENC_VMCS_COMPONENT_FULL_32 (0, vm_exit)
#define VMCS_RO_EXIT_REASON \
   ENC_VMCS_COMPONENT_FULL_32 (1, vm_exit)
#define VMCS_RO_VMEXIT_INTERRUPTION_INFO \
   ENC_VMCS_COMPONENT_FULL_32 (2, vm_exit)
#define VMCS_RO_VMEXIT_INTERRUPTION_ERROR_CODE \
   ENC_VMCS_COMPONENT_FULL_32 (3, vm_exit)
#define VMCS_RO_IDT_VECTORING_INFO_FIELD \
   ENC_VMCS_COMPONENT_FULL_32 (4, vm_exit)
#define VMCS_RO_IDT_VECTORING_ERROR_CODE \
   ENC_VMCS_COMPONENT_FULL_32 (5, vm_exit)
#define VMCS_RO_VMEXIT_INSTRUCTION_LENGTH \
   ENC_VMCS_COMPONENT_FULL_32 (6, vm_exit)
#define VMCS_RO_VMEXIT_INSTRUCTION_INFO \
   ENC_VMCS_COMPONENT_FULL_32 (7, vm_exit)

// 32-bit fields (guest-state)
#define VMCS_GUEST_ES_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (0, guest_state)
#define VMCS_GUEST_CS_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (1, guest_state)
#define VMCS_GUEST_SS_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (2, guest_state)
#define VMCS_GUEST_DS_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (3, guest_state)
#define VMCS_GUEST_FS_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (4, guest_state)
#define VMCS_GUEST_GS_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (5, guest_state)
#define VMCS_GUEST_LDTR_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (6, guest_state)
#define VMCS_GUEST_TR_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (7, guest_state)
#define VMCS_GUEST_GDTR_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (8, guest_state)
#define VMCS_GUEST_IDTR_LIMIT \
   ENC_VMCS_COMPONENT_FULL_32 (9, guest_state)
#define VMCS_GUEST_ES_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (10, guest_state)
#define VMCS_GUEST_CS_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (11, guest_state)
#define VMCS_GUEST_SS_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (12, guest_state)
#define VMCS_GUEST_DS_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (13, guest_state)
#define VMCS_GUEST_FS_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (14, guest_state)
#define VMCS_GUEST_GS_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (15, guest_state)
#define VMCS_GUEST_LDTR_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (16, guest_state)
#define VMCS_GUEST_TR_ACCESS_RIGHTS \
   ENC_VMCS_COMPONENT_FULL_32 (17, guest_state)
#define VMCS_GUEST_INTERRUPTIBILITY_STATE \
   ENC_VMCS_COMPONENT_FULL_32 (18, guest_state)
#define VMCS_GUEST_ACTIVITY_STATE \
   ENC_VMCS_COMPONENT_FULL_32 (19, guest_state)
#define VMCS_GUEST_SMBASE \
   ENC_VMCS_COMPONENT_FULL_32 (20, guest_state)
#define VMCS_GUEST_IA32_SYSENTER_CS \
   ENC_VMCS_COMPONENT_FULL_32 (21, guest_state)
#define VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE \
   ENC_VMCS_COMPONENT_FULL_32 (23, guest_state)

// 32-bit fields (host-state)
#define VMCS_HOST_IA32_SYSENTER_CS \
   ENC_VMCS_COMPONENT_FULL_32 (0, host_state)

// natural-width fields (control)
#define VMCS_CTRL_CR0_GUEST_HOST_MASK \
   ENC_VMCS_COMPONENT_FULL_NAT (0, control)
#define VMCS_CTRL_CR4_GUEST_HOST_MASK \
   ENC_VMCS_COMPONENT_FULL_NAT (1, control)
#define VMCS_CTRL_CR0_READ_SHADOW \
   ENC_VMCS_COMPONENT_FULL_NAT (2, control)
#define VMCS_CTRL_CR4_READ_SHADOW \
   ENC_VMCS_COMPONENT_FULL_NAT (3, control)
#define VMCS_CTRL_CR3_TARGET_VALUE_0 \
   ENC_VMCS_COMPONENT_FULL_NAT (4, control)
#define VMCS_CTRL_CR3_TARGET_VALUE_1 \
   ENC_VMCS_COMPONENT_FULL_NAT (5, control)
#define VMCS_CTRL_CR3_TARGET_VALUE_2 \
   ENC_VMCS_COMPONENT_FULL_NAT (6, control)
#define VMCS_CTRL_CR3_TARGET_VALUE_3 \
   ENC_VMCS_COMPONENT_FULL_NAT (7, control)

// natural-width fields (read-only data)
#define VMCS_RO_EXIT_QUALIFICATION \
   ENC_VMCS_COMPONENT_FULL_NAT (0, vm_exit)
#define VMCS_RO_IO_RCX \
   ENC_VMCS_COMPONENT_FULL_NAT (1, vm_exit)
#define VMCS_RO_IO_RSI \
   ENC_VMCS_COMPONENT_FULL_NAT (2, vm_exit)
#define VMCS_RO_IO_RDI \
   ENC_VMCS_COMPONENT_FULL_NAT (3, vm_exit)
#define VMCS_RO_IO_RIP \
   ENC_VMCS_COMPONENT_FULL_NAT (4, vm_exit)
#define VMCS_RO_GUEST_LINEAR_ADDRESS \
   ENC_VMCS_COMPONENT_FULL_NAT (5, vm_exit)

// natural-width fields (guest-state)
#define VMCS_GUEST_CR0 \
   ENC_VMCS_COMPONENT_FULL_NAT (0, guest_state)
#define VMCS_GUEST_CR3 \
   ENC_VMCS_COMPONENT_FULL_NAT (1, guest_state)
#define VMCS_GUEST_CR4 \
   ENC_VMCS_COMPONENT_FULL_NAT (2, guest_state)
#define VMCS_GUEST_ES_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (3, guest_state)
#define VMCS_GUEST_CS_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (4, guest_state)
#define VMCS_GUEST_SS_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (5, guest_state)
#define VMCS_GUEST_DS_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (6, guest_state)
#define VMCS_GUEST_FS_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (7, guest_state)
#define VMCS_GUEST_GS_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (8, guest_state)
#define VMCS_GUEST_LDTR_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (9, guest_state)
#define VMCS_GUEST_TR_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (10, guest_state)
#define VMCS_GUEST_GDTR_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (11, guest_state)
#define VMCS_GUEST_IDTR_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (12, guest_state)
#define VMCS_GUEST_DR7 \
   ENC_VMCS_COMPONENT_FULL_NAT (13, guest_state)
#define VMCS_GUEST_RSP \
   ENC_VMCS_COMPONENT_FULL_NAT (14, guest_state)
#define VMCS_GUEST_RIP \
   ENC_VMCS_COMPONENT_FULL_NAT (15, guest_state)
#define VMCS_GUEST_RFLAGS \
   ENC_VMCS_COMPONENT_FULL_NAT (16, guest_state)
#define VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS \
   ENC_VMCS_COMPONENT_FULL_NAT (17, guest_state)
#define VMCS_GUEST_IA32_SYSENTER_ESP \
   ENC_VMCS_COMPONENT_FULL_NAT (18, guest_state)
#define VMCS_GUEST_IA32_SYSENTER_EIP \
   ENC_VMCS_COMPONENT_FULL_NAT (19, guest_state)
#define VMCS_GUEST_IA32_S_CET \
   ENC_VMCS_COMPONENT_FULL_NAT (20, guest_state)
#define VMCS_GUEST_SSP \
   ENC_VMCS_COMPONENT_FULL_NAT (21, guest_state)
#define VMCS_GUEST_IA32_INTERRUPT_SSP_TABLE_ADDR \
   ENC_VMCS_COMPONENT_FULL_NAT (22, guest_state)

// natural-width fields (host-state)
#define VMCS_HOST_CR0 \
   ENC_VMCS_COMPONENT_FULL_NAT (0, host_state)
#define VMCS_HOST_CR3 \
   ENC_VMCS_COMPONENT_FULL_NAT (1, host_state)
#define VMCS_HOST_CR4 \
   ENC_VMCS_COMPONENT_FULL_NAT (2, host_state)
#define VMCS_HOST_FS_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (3, host_state)
#define VMCS_HOST_GS_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (4, host_state)
#define VMCS_HOST_TR_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (5, host_state)
#define VMCS_HOST_GDTR_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (6, host_state)
#define VMCS_HOST_IDTR_BASE \
   ENC_VMCS_COMPONENT_FULL_NAT (7, host_state)
#define VMCS_HOST_IA32_SYSENTER_ESP \
   ENC_VMCS_COMPONENT_FULL_NAT (8, host_state)
#define VMCS_HOST_IA32_SYSENTER_EIP \
   ENC_VMCS_COMPONENT_FULL_NAT (9, host_state)
#define VMCS_HOST_RSP \
   ENC_VMCS_COMPONENT_FULL_NAT (10, host_state)
#define VMCS_HOST_RIP \
   ENC_VMCS_COMPONENT_FULL_NAT (11, host_state)
#define VMCS_HOST_IA32_S_CET \
   ENC_VMCS_COMPONENT_FULL_NAT (12, host_state)
#define VMCS_HOST_SSP \
   ENC_VMCS_COMPONENT_FULL_NAT (13, host_state)
#define VMCS_HOST_IA32_INTERRUPT_SSP_TABLE_ADDR \
   ENC_VMCS_COMPONENT_FULL_NAT (14, host_state)

typedef enum
{
   activity_state_active = 0,
   activity_state_hlt,
   activity_state_shutdown,
   activity_state_wait_for_sipi
} __vmcs_guest_activity_state;

typedef union
{
   u32 all;
   struct
   {
      u32 blocking_by_sti        : 1;
      u32 blocking_by_mov_ss     : 1;
      u32 blocking_by_smi        : 1;
      u32 blocking_by_nmi        : 1;
      u32 enclave_interruption   : 1;
      u32 reserved_0             : 27;
   } fields;
} __vmcs_guest_interruptibility_state;

typedef union
{
   u64 all;
   struct
   {
      u64 b3_b0               : 4;
      u64 reserved_0          : 7;
      u64 bld                 : 1;
      u64 enabled_breakpoint  : 1;
      u64 reserved_1          : 1;
      u64 bs                  : 1;
      u64 reserved_2          : 1;
      u64 rtm                 : 1;
      u64 reserved_3          : 47;
   } fields;
} __vmcs_guest_pending_debug_exceptions;

#endif // __LKM_ENC_H__
