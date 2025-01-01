#include <linux/linkage.h>

.extern cpu_init_main

.extern vmexit_handler

.extern vmresume_error_handler

.macro push_gp
   pushq %rbp
   subq $8, %rsp
   pushq %rax
   pushq %rbx
   pushq %rcx
   pushq %rdx
   pushq %rsi
   pushq %rdi
   pushq %r8
   pushq %r9
   pushq %r10
   pushq %r11
   pushq %r12
   pushq %r13
   pushq %r14
   pushq %r15
.endm

.macro pop_gp
   popq %r15
   popq %r14
   popq %r13
   popq %r12
   popq %r11
   popq %r10
   popq %r9
   popq %r8
   popq %rdi
   popq %rsi
   popq %rdx
   popq %rcx
   popq %rbx
   popq %rax
   addq $8, %rsp
   popq %rbp
.endm

.text

// per-cpu initialization routine
SYM_FUNC_START(cpu_init_asm)
   pushfq
   push_gp
   // %rdi: cpu_ctx struct for this processor
   // %rsi: original stack pointer (GUEST_RSP)
   // %rdx: guest resume address (GUEST_RIP)
   movq $0, %rdi
   movq $0, %rsi
   movq $0, %rdx
   // call vmx setup routine
   subq $0x20, %rsp
   call cpu_init_main
   addq $0x20, %rsp
   // execution continues here in vmx non-root mode after VMLAUNCH.
   // if VMLAUNCH fails to execute, execution continues here outside VMX
   // operation with error code in RFLAGS
.cpu_init_post:
   pop_gp
   popfq
   RET
SYM_FUNC_END(cpu_init_asm)
EXPORT_SYMBOL(cpu_init_asm)

// vm-exit handler entry point
SYM_FUNC_START(vmexit_start)
   push_gp
   // call vm-exit handler routine
   subq $0x20, %rsp
   call vmexit_handler
   addq $0x20, %rsp
   test %al, %al
   pop_gp
   jnz .err
   VMRESUME
.err:
   // call vmresume error handler routine
   subq $0x20, %rsp
   call vmresume_error_handler
   addq $0x20, %rsp
.crit_err:
   // todo: implement graceful hypervisor shutdown
   hlt
   jmp .crit_err
SYM_FUNC_END(vmexit_start)
EXPORT_SYMBOL(vmexit_start)
