#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "town.h"

struct town {
	char name[32];
	int x;
	int y;
};

town* town_create(const char* name, int x, int y)
{
	town* t = malloc(sizeof(town));
	assert(t);

	memset(t, 0x00, sizeof(*t));
	t->x = x;
	t->y = y;
	strncpy(t->name, name, 31);

	return t;
}

void town_cleanup(town* t)
{
	free(t);
}

void town_get_location(const town* t, int* x, int* y)
{
	*x = t->x;
	*y = t->y;
}

const char* town_get_name(const town* t)
{
	return t->name;
}

