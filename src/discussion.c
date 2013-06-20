#include "xmalloc.h"
#include "discussion.h"

struct discussion {
	int x;
};

discussion* discussion_create(void)
{
	discussion* d = xmalloc(sizeof(discussion));
	return d;
}

void discussion_cleanup(discussion* d)
{
	xfree(d);
}


