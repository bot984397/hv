#ifndef __LKM_VMM_H__
#define __LKM_VMM_H__

#include <linux/types.h>
#include <linux/gfp.h>

#include "cpu.h"

int vmm_new (void);

void vmm_del (void);

#endif // __LKM_VMM_H__
