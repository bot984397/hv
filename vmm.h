#ifndef __LKM_VMM_H__
#define __LKM_VMM_H__

#include <linux/types.h>
#include <linux/gfp.h>

#include "cpu.h"

vmm_ctx_t* vmm_alloc (void);
vmm_ctx_t* vmm_init (void);

void vmm_free (vmm_ctx_t *vmm_ctx);

#endif // __LKM_VMM_H__
