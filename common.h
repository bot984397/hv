#ifndef __LKM_COMMON_H__
#define __LKM_COMMON_H__

#define LKM_DEBUG

#ifdef LKM_DEBUG
#define LOG_DBG(fmt, ...) \
   pr_info ("[lkm] %s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DBG(fmt, ...)
#endif

#include <linux/types.h>
#include <linux/atomic.h>

#include "msr.h"

typedef struct _vmm_ctx_t vmm_ctx_t;
typedef struct _vcpu_ctx_t vcpu_ctx_t;
typedef union _vm_region_t vm_region_t;

struct _vmm_ctx_t 
{
   int vcpu_max;
   atomic_t vcpu_init;
   vcpu_ctx_t **vcpu_ctxs;
};

struct _vcpu_ctx_t 
{
   vmm_ctx_t *vmm_ctx;

   vm_region_t *vmxon_region;
   u64 vmxon_physical;
   vm_region_t *vmcs_region;
   u64 vmcs_physical;

   struct
   {
      u8 io_bitmap_a[4096];
      u8 io_bitmap_b[4096];
      u8 msr_bitmap_a[4096];
      u8 msr_bitmap_b[4096];
   } bitmaps;

   struct
   {
      ia32_vmx_basic_t vmx_basic;
   } cached;
};

union _vm_region_t
{
   unsigned char data[4096];
   struct
   {
      u32 rev_ident : 31;
      u32 reserved_0 : 1;
      u32 abort_indicator;
   } reserved;
} __attribute__((packed));

#endif // __LKM_COMMON_H__
