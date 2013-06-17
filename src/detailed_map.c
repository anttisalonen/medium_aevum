#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "detailed_map.h"

struct detmap {
	int x;
};

detmap* detmap_create()
{
	detmap* m = malloc(sizeof(detmap));
	assert(m);

	memset(m, 0x00, sizeof(*m));

	return m;
}

void detmap_cleanup(detmap* m)
{
	free(m);
}

detterrain_type detmap_get_terrain_at(const detmap* m, int x, int y)
{
	if((x % 5 == 0) && (y % 7 == 0)) {
		return dett_tree;
	} else if((x % 3 == 0) && (y % 13 == 0)) {
		return dett_water;
	} else if((x % 17 == 0) && (y % 9 == 0)) {
		return dett_rock;
	} else {
		return dett_grass;
	}
}

