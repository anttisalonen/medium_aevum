#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "mapinfo.h"

struct map {
	int x;
};

map* map_create()
{
	map* m = malloc(sizeof(map));
	assert(m);

	memset(m, 0x00, sizeof(*m));

	return m;
}

void map_cleanup(map* m)
{
	free(m);
}

terrain_type map_get_terrain_at(const map* m, int x, int y)
{
	if((x % 10000 == 0) || (y % 10000 == 0)) {
		return tt_hills;
	} else if((x % 5 == 0) || (y % 7 == 0)) {
		return tt_forest;
	} else if(x % 3 == 0) {
		return tt_sea;
	} else {
		return tt_grass;
	}
}

