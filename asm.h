#ifndef __LKM_ASM_H__
#define __LKM_ASM_H__

#include <linux/types.h>

static inline __attribute__((always_inline)) int __vmx_on (u64 phys)
{
   int res = 0;
   __asm__ volatile
   (
      "vmxon %[region];"
      "setc %b0;"
      : "+q" (res)
      : [region] "m" (phys)
      : "cc"
   );
   return res;
}

static inline __attribute__((always_inline)) void __vmx_off (void)
{
   __asm__ volatile
   (
      "vmxoff;"
      :
      :
      : "cc"
   );
}

static inline __attribute__((always_inline)) int __vmx_vmclear (u64 phys)
{
   int res = 0;
   __asm__ volatile
   (
      "vmclear %[region];"
      "setc %b0;"
      : "+q" (res)
      : [region] "m" (phys)
      : "cc"
   );
   return res;
}

static inline __attribute__((always_inline)) int __vmx_vmptrld (u64 phys)
{
   int res = 0;
   __asm__ volatile
   (
      "vmptrld %[region];"
      "setc %b0;"
      : "+q" (res)
      : [region] "m" (phys)
      : "cc"
   );
   return res;
}

static inline __attribute__((always_inline)) 
void __vmx_vmwrite (u64 field, u64 value)
{
   __asm__ volatile
   (
      "vmwrite %[value], %[field];"
      :
      : [field] "r" (field),
        [value] "r" (value)
      : "cc"
   );
}

static inline __attribute__((always_inline)) u64 __vmx_vmread (u64 field)
{
   u64 value = 0;
   __asm__ volatile
   (
      "vmread %[value], %[field];"
      : [value] "=r" (value)
      : [field] "r" (field)
      : "cc"
   );
   return value;
}

static inline __attribute__((always_inline)) int __vmx_vmlaunch (void)
{
   int res = -1;
   __asm__ volatile
   (
      "vmlaunch;"
      "setc %b0;"
      : "+q" (res)
      :
      : "cc"
   );
   return res;
}

static inline __attribute__((always_inline)) u64 __read_dr (unsigned int reg)
{
   u64 value = 0;

   switch (reg)
   {
      case 0:
         __asm__ volatile ("mov %%dr0, %0" : "=r" (value));
         break;
      case 1:
         __asm__ volatile ("mov %%dr1, %0" : "=r" (value));
         break;
      case 2: 
         __asm__ volatile ("mov %%dr2, %0" : "=r" (value));
         break;
      case 3:
         __asm__ volatile ("mov %%dr3, %0" : "=r" (value));
         break;
      case 6:
         __asm__ volatile ("mov %%dr6, %0" : "=r" (value));
         break;
      case 7:
         __asm__ volatile ("mov %%dr7, %0" : "=r" (value));
         break;
      default:
         return 0;
   }

   return value;
}

static inline __attribute__((always_inline)) u64 __read_rflags (void)
{
   u64 value = 0;
   __asm__ volatile
   (
      "pushfq;"
      "popq %0;"
      : "=r" (value)
      :
      : "memory"
   );
   return value;
}

static inline __attribute__((always_inline)) u16 __read_ldtr (void)
{
   u16 v;
   __asm__ volatile
   (
      "sldt %0"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __read_tr (void)
{
   u16 v;
   __asm__ volatile
   (
      "str %0"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __read_cs (void)
{
   u16 v;
   __asm__ volatile
   (
      "mov %0, cs"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __read_ss (void)
{
   u16 v;
   __asm__ volatile
   (
      "mov %0, ss"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __read_ds (void)
{
   u16 v;
   __asm__ volatile
   (
      "mov %0, ds"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __read_es (void)
{
   u16 v;
   __asm__ volatile
   (
      "mov %0, es"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __read_fs (void)
{
   u16 v;
   __asm__ volatile
   (
      "mov %0, fs"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __read_gs (void)
{
   u16 v;
   __asm__ volatile
   (
      "mov %0, gs"
      : "=r" (v)
   );
   return v;
}

static inline __attribute__((always_inline)) u16 __seglimit (u16 sel)
{
   u16 v;
   __asm__ volatile
   (
      "lsl ;"
      "setz %b0;"
      : "+q" (v)
   );
   return v;
}

#endif // __LKM_ASM_H__
