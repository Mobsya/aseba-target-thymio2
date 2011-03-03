#include "regulator.h"

#include <types/types.h>

static unsigned int refcount;

// There is a thiny race window when VA is disabled, and at the same time is preempted by a va_get
void va_get(void) {
	atomic_add(&refcount,1);
	VA_ENABLE = 1;
}

void va_put(void) {
	atomic_add(&refcount, -1);
	if(!refcount)
		VA_ENABLE = 0;	
}

