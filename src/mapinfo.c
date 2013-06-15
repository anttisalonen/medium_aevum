#include <stdlib.h>

#include "mapinfo.h"

struct map {
	int x;
};

map* map_create()
{
	return malloc(sizeof(map));
}

void map_cleanup(map* m)
{
	free(m);
}

terrain_type get_terrain_at(const map* m, int x, int y)
{
	if((x % 5 == 0) || (y % 7 == 0)) {
		return tt_forest;
	} else if(x % 3 == 0) {
		return tt_sea;
	}
	else {
		return tt_grass;
	}
}

