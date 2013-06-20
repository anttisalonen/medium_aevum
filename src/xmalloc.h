#ifndef MA_XMALLOC_H
#define MA_XMALLOC_H

#include <stdlib.h>

void *xmalloc(size_t size);
void xfree(void *ptr);

#endif

