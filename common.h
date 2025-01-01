#ifndef __LKM_COMMON_H__
#define __LKM_COMMON_H__

#define LKM_DEBUG

#if defined(NTDDI_VERSION) || defined (_KERNEL_MODE) || defined(DRIVER)
#define PLATFORM_WINDOWS
#elif defined(__KERNEL__)
#define PLATFORM_LINUX
#endif

#ifndef size_assert
#define size_assert(obj, size) \
   _Static_assert(sizeof(obj)==size,"size mismatch: " #obj)
#endif // size_assert

#ifdef LKM_DEBUG
#define LOG_DBG(fmt, ...) \
   pr_info ("[lkm] %s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DBG(fmt, ...)
#endif

#define VMX_OK 0
#define VMX_ERR(fn) ((fn) != VMX_OK)

#define BITS(v) v / 8

#define _always_inline_ inline __attribute__((always_inline))
#define _warn_unused_ __attribute__((warn_unused_result))

#define _unused_ __attribute__((__unused__))

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/atomic.h>

#include "msr.h"

typedef struct _vmm_ctx vmm_ctx;
typedef struct _cpu_ctx cpu_ctx;
typedef struct _vmx_reg vmx_reg;

struct _vmm_ctx
{
   int vcpu_max;
   atomic_t vcpu_init;
   
   cpu_ctx **vcpu_ctxs;
};

struct _cpu_ctx
{
   vmm_ctx *vmm_ctx;
   u32 cpu_num;

   vmx_reg *vmxon_region;
   u64 vmxon_physical;

   vmx_reg *vmcs_region;
   u64 vmcs_physical;

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
      ia32_vmx_basic_t vmx_basic;
   } cached;
};

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
   u8 data[];
} __attribute__((packed));

extern vmm_ctx *g_vmm_ctx;

#endif // __LKM_COMMON_H__
