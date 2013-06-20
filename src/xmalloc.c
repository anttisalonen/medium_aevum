#include <stdlib.h>
#include <string.h>
#include <assert.h>


void *xmalloc(size_t size)
{
	void* ptr = malloc(size);
	assert(ptr);

	memset(ptr, 0x00, size);

	return ptr;
}

void xfree(void *ptr)
{
	free(ptr);
}

