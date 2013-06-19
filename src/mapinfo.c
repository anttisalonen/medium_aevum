#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "mapinfo.h"

struct map {
	town* town;
};

map* map_create(void)
{
	map* m = malloc(sizeof(map));
	assert(m);

	memset(m, 0x00, sizeof(*m));

	m->town = town_create();

	return m;
}

void map_cleanup(map* m)
{
	town_cleanup(m->town);
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

const town* map_get_town_at(const map* m, int x, int y)
{
	if(x == 10003 && y == 20005)
		return m->town;
	else
		return NULL;
}


