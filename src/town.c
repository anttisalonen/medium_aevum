#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "town.h"

struct town {
	int x;
};

town* town_create()
{
	town* t = malloc(sizeof(town));
	assert(t);

	memset(t, 0x00, sizeof(*t));

	return t;
}

void town_cleanup(town* t)
{
	free(t);
}


