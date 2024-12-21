#ifndef __LKM_VMM_H__
#define __LKM_VMM_H__

#include <linux/types.h>
#include <linux/gfp.h>

#include "cpu.h"

bool vmm_alloc (void);

void vmm_free (void);

#endif // __LKM_VMM_H__
