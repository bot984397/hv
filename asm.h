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
         asm volatile ("mov %%dr0, %0" : "=r" (value));
         break;
      case 1:
         asm volatile ("mov %%dr1, %0" : "=r" (value));
         break;
      case 2: 
         asm volatile ("mov %%dr2, %0" : "=r" (value));
         break;
      case 3:
         asm volatile ("mov %%dr3, %0" : "=r" (value));
         break;
      case 6:
         asm volatile ("mov %%dr6, %0" : "=r" (value));
         break;
      case 7:
         asm volatile ("mov %%dr7, %0" : "=r" (value));
         break;
      default:
         return 0;
   }

   return value;
}

static inline __attribute__((always_inline)) u64 __read_dr7 (void)
{
   u64 value = 0;
   __asm__ volatile
   (
      "mov %%dr7, %0"
      : "=r" (value)
   );
   return value;
}

static inline __attribute__((always_inline)) void __write_dr7 (u64 value)
{
   __asm__ volatile
   (
      "mov %0, %%dr7"
      :
      : "r" (value)
   );
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

#endif // __LKM_ASM_H__
