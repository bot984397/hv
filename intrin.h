#ifndef __LKM_ASM_H__
#define __LKM_ASM_H__

#include <linux/types.h>
#include "common.h"
#include "vmcs.h"

static _always_inline_ u64 readcr0 (void)
{
   u64 r;
   __asm__ __volatile__ 
   (
      "mov %%cr0, %[r]"
      : [r] "=r" (r)
      :
      : "memory"
   );
   return r;
}

static _always_inline_ void writecr0 (u64 v)
{
   __asm__ __volatile__
   (
      "mov %[v], %%cr0"
      :
      : [v] "r" (v)
      : "memory"
   );
}

static _always_inline_ u64 readcr3 (void)
{
   u64 r;
   __asm__ __volatile__
   (
      "mov %%cr3, %[r]"
      : [r] "=r" (r)
      :
      : "memory"
   );
   return r;
}

static _always_inline_ void writecr3 (u64 v)
{
   __asm__ __volatile__
   (
      "mov %[v], %%cr3"
      :
      : [v] "r" (v)
      : "memory"
   );
}

static _always_inline_ u64 readcr4 (void)
{
   u64 r;
   __asm__ __volatile__
   (
      "mov %%cr4, %[r]"
      : [r] "=r" (r)
      :
      : "memory"
   );
   return r;
}

static _always_inline_ void writecr4 (u64 v)
{
   __asm__ __volatile__
   (
      "mov %[v], %%cr4"
      :
      : [v] "r" (v)
      : "memory"
   );
}

static _always_inline_ void wrmsr (u32 m, u64 v)
{
   u32 l = v & 0xFFFFFFFF;
   u32 h = v >> 32;
   __asm__ __volatile__
   (
      "wrmsr"
      :
      : "c" (m), "a" (l), "d" (h)
      : "memory"
   );
}

static _always_inline u64 rdmsr (u32 m)
{
   u32 l, h;
   rdmsr1 (m, &l, &h);
   return ((u64)h << 32) | l;
}

static _always_inline_ void rdmsr1 (u32 m, u32 *vl, u32 *vh)
{
   u32 l, h;
   __asm__ __volatile__
   (
      "rdmsr"
      : "=a" (l), "=d" (h)
      : "c" (m)
      : "memory"
   );
   *vl = l;
   *vh = h;
}

static _always_inline_ _cpuid cpuid1 (u32 l, u32 *a, u32 *b, u32 *c, u32 *d)
{
   __asm__ __volatile__
   (
      "cpuid"
      : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
      : "a" (l)
      :
   );
}

static _always_inline_ u8 vmxon (u64 p)
{
   u8 r;
   __asm__ __volatile__
   (
      "vmxon %[p]; setna %[r]"
      : [r] "=rm" (r)
      : [p] "m" (p)
      : "cc", "memory"
   );
   return r;
}

static _always_inline_ u8 vmxoff (void)
{
   u8 r;
   __asm__ __volatile__
   (
      "vmxoff; setna %[r]"
      : [r] "=rm" (r)
      :
      : "cc", "memory"
   );
   return r;
}

static _always_inline_ u8 vmclear (u64 p)
{
   u8 r;
   __asm__ __volatile__
   (
      "vmclear %[p]; setna %[r]"
      : [r] "=rm" (r)
      : [p] "m" (p)
      : "cc", "memory"
   );
   return r;
}

static _always_inline_ u8 vmptrld (u64 p)
{
   u8 r;
   __asm__ __volatile__
   (
      "vmptrld %[p]; setna %[r]"
      : [r] "=rm" (r)
      : [p] "m" (p)
      : "cc", "memory"
   );
   return r;
}

static _always_inline_ u8 vmwrite (u64 f, u64 v)
{
   u8 r;
   __asm__ __volatile__
   (
      "vmwrite %[v], %[f]; setna %[r]"
      : [r] "=rm" (r)
      : [v] "rm" (v), [f] "r" (f)
      : "cc", "memory"
   );
   return r;
}

static _always_inline_ u8 vmread (u64 f, u64 *v)
{
   u8 r;
   __asm__ __volatile__
   (
      "vmread %%rax, %[f]; setna %[r]"
      : [r] "=r" (r), "=a" (*v)
      : [f] "r" (f)
      : "cc", "memory"
   );
   return r;
}

static _always_inline_ u64 fvmread (u64 f)
{
   u64 v;
   vmread (f, &v);
   return v;
}

static _always_inline_ u8 vmlaunch (void)
{
   u8 r;
   __asm__ __volatile__
   (
      "vmlaunch; setna %[r]"
      : [r] "=r" (r)
      :
      : "cc", "memory"
   );
   return r;
}

static _always_inline_ u64 __read_dr (u8 reg)
{
   u64 v = 0;
   switch (reg)
   {
      case 0:
         __asm__ __volatile__ ("mov %%dr0, %[v]" : [v] "=r" (v));
         break;
      case 1:
         __asm__ __volatile__ ("mov %%dr1, %[v]" : [v] "=r" (v));
         break;
      case 2: 
         __asm__ __volatile__ ("mov %%dr2, %[v]" : [v] "=r" (v));
         break;
      case 3:
         __asm__ __volatile__ ("mov %%dr3, %[v]" : [v] "=r" (v));
         break;
      case 6:
         __asm__ __volatile__ ("mov %%dr6, %[v]" : [v] "=r" (v));
         break;
      case 7:
         __asm__ __volatile__ ("mov %%dr7, %[v]" : [v] "=r" (v));
         break;
      default:
         break;
   }
   return v;
}

static _always_inline_ u64 read_rflags (void)
{
   u64 v;
   __asm__ __volatile__
   (
      "pushfq; popq %0"
      : "=r" (v)
      :
      : "memory"
   );
   return v;
}

static _always_inline_ u16 read_ldtr (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "sldt %0"
      : "=r" (v)
   );
   return v;
}

static _always_inline_ u16 read_tr (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "str %[v]"
      : [v] "=rm" (v)
   );
   return v;
}

static _always_inline_ u16 read_cs (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "mov %%cs, %[v]"
      : [v] "=rm" (v)
   );
   return v;
}

static _always_inline_ u16 read_ss (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "mov %%ss, %[v]"
      : [v] "=rm" (v)
   );
   return v;
}

static _always_inline_ u16 read_ds (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "mov %%ds, %[v]"
      : [v] "=rm" (v)
   );
   return v;
}

static _always_inline_ u16 read_es (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "mov %%es, %[v]"
      : [v] "=rm" (v)
   );
   return v;
}

static _always_inline_ u16 read_fs (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "mov %%fs, %[v]"
      : [v] "=rm" (v)
   );
   return v;
}

static _always_inline_ u16 read_gs (void)
{
   u16 v;
   __asm__ __volatile__
   (
      "mov %%gs, %[v]"
      : [v] "=rm" (v)
   );
   return v;
}

static _always_inline_ u32 seglimit (u32 s)
{
   u32 v;
   __asm__ __volatile__
   (
      "lsl %[v], %[s]"
      : [v] "=r" (v)
      : [s] "rm" (s)
      : "cc", "memory"
   );
   return v;
}

static _always_inline_ u32 lar (u32 s)
{
   u32 v;
   __asm__ __volatile__
   (
      "lar %[v], %[s]\n\t"
      "jz 1f\n\t"
      "xor %[v], %[v]\n\t"
      "1:"
      : [v] "=r" (v)
      : [s] "r" (s)
      : "cc"
   );
   return v;
}

static _always_inline_ __pseudo_descriptor sgdt (void)
{
   __pseudo_descriptor v;
   __asm__ __volatile__
   (
      "sgdt %[v]"
      : [v] "=m" (v)
   );
   return v;
}

static _always_inline_ __pseudo_descriptor sidt (void)
{
   __pseudo_descriptor v;
   __asm__ __volatile__
   (
      "sidt %[v]"
      : [v] "=m" (v)
   );
   return v;
}

#endif // __LKM_ASM_H__
