#ifndef __LKM_COMMON_H__
#define __LKM_COMMON_H__

#define LKM_DEBUG

#if defined(NTDDI_VERSION) || defined (_KERNEL_MODE) || defined(DRIVER)
#define PLATFORM_WINDOWS
#elif defined(__KERNEL__)
#define PLATFORM_LINUX
#endif

#include <linux/version.h>
#define OLD_CPUHOTPLUG  (LINUX_VERSION_CODE <= KERNEL_VERSION(4,8,0))
#define KMALLOC_ALIGNED (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))

#ifndef size_assert
#define size_assert(obj, size) \
   _Static_assert(sizeof(obj)==size,"size mismatch: " #obj)
#endif // size_assert

#ifdef LKM_DEBUG
#define LOG_DBG(fmt, ...) \
   pr_info ("[lkm] %s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DBG(fmt, ...)
#endif // LKM_DEBUG

#define VMX_OK 0
#define VMX_ERR(fn) ((fn) != VMX_OK)

#define BITS(v) v / 8
#define BYTES(v) v * 8

#define LOBITS64(v) (u32)v
#define HIBITS64(v) (u32)(v >> 32)
#define LOBITS32(v) (u16)v
#define HIBITS32(v) (u16)(v >> 16)
#define LOBITS16(v) (u8)v
#define HIBITS16(v) (u8)(v >> 8)

#if KMALLOC_ALIGNED
#define VMM_HOST_STACK_SIZE PAGE_SIZE * 6
#else
#define VMM_HOST_STACK_SIZE PAGE_SIZE * 8
#endif // KMALLOC_ALIGNED

#define _always_inline_ inline __attribute__((always_inline))
#define _warn_unused_result_ __attribute__((warn_unused_result))
#define _packed_ __attribute__((packed))
#define _unused_ __attribute__((__unused__))

#define EVMCLEAR  80
#define EVMPTRLD  81
#define EVMLAUNCH 82
#define EVMSTATE  83

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/atomic.h>

#include "arch.h"

typedef struct _vmm_ctx vmm_ctx;
typedef struct _cpu_ctx cpu_ctx;
typedef struct _vmx_reg vmx_reg;
typedef struct _vmm_stk vmm_stk;

struct _vmm_ctx
{
   int cpu_on;
   atomic_t cpu_init;
   
   cpu_ctx **_cpu_ctx;
};

struct _cpu_ctx
{
   u32 cpu_num;

   vmx_reg *vmxon_region;
   u64 vmxon_physical;

   vmx_reg *vmcs_region;
   u64 vmcs_physical;

   vmm_stk *_vmm_stk;

   struct
   {
      u8 *io_bitmap_a;
      u64 io_bitmap_a_phys;

      u8 *io_bitmap_b;
      u64 io_bitmap_b_phys;

      u8 *msr_bitmaps;
      u64 msr_bitmaps_phys;
   } bitmaps;

   struct
   {
      ia32_vmx_basic vmx_basic;
   } cached;

   u8 launched;
};

struct _vmm_stk
{
   u8 data[VMM_HOST_STACK_SIZE - sizeof (cpu_ctx *)];
   cpu_ctx *_cpu_ctx;
};
size_assert (vmm_stk, VMM_HOST_STACK_SIZE);

struct _vmx_reg
{
   union
   {
      u32 ctl;
      struct
      {
         u32 rev_ident : 31;
         u32 reserved_0 : 1;
      };
   } header;
   u32 abort_indicator;
   u8 data[4096 - sizeof (u64)];
} __attribute__((packed));
size_assert (vmx_reg, 4096);

typedef struct
{
   u64 sp;
   u64 rf;
   u64 ip;
} guest_init;

extern vmm_ctx *g_vmm_ctx;

#endif // __LKM_COMMON_H__
