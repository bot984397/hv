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
   typedef char obj##__size_assert_[(!!(sizeof(obj)==size))*2-1]
#endif

#ifdef LKM_DEBUG
#define LOG_DBG(fmt, ...) \
   pr_info ("[lkm] %s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DBG(fmt, ...)
#endif

#define VMX_OK 0
#define VMX_ERR(fn) ((fn) != 0)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/atomic.h>

#include "msr.h"

#define __read_cr0 read_cr0
#define __write_cr0 write_cr0

typedef struct _vmm_ctx_t vmm_ctx_t;
typedef struct _vcpu_ctx_t vcpu_ctx_t;
typedef struct _vm_region_t vm_region_t;

struct _vmm_ctx_t 
{
   int vcpu_max;
   atomic_t vcpu_init;
   vcpu_ctx_t **vcpu_ctxs;

   vcpu_ctx_t *vcpu_ctx_ll;
};

struct _vcpu_ctx_t 
{
   vmm_ctx_t *vmm_ctx;
   u32 cpu_num;

   vm_region_t *vmxon_region;
   u64 vmxon_physical;

   vm_region_t *vmcs_region;
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

   vcpu_ctx_t *flink;
   vcpu_ctx_t *blink;
};

struct _vm_region_t
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
size_assert (vm_region_t, 4096);

extern vmm_ctx_t *g_vmm_ctx;

#endif // __LKM_COMMON_H__
