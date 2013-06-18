#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "detailed_map.h"

struct detmap {
	terrain_type terrain;
	const town* town;
};

detmap* detmap_create(terrain_type tt, const town* t)
{
	detmap* m = malloc(sizeof(detmap));
	assert(m);

	memset(m, 0x00, sizeof(*m));
	m->terrain = tt;
	m->town = t;

	return m;
}

void detmap_cleanup(detmap* m)
{
	free(m);
}

detterrain_type detmap_get_terrain_at(const detmap* m, int x, int y)
{
	if(m->town) {
		if(x > 510 && x < 520) {
			if(y > 505 && y < 515) {
				if(x == 511 || x == 519 || y == 506 || y == 514) {
					if(x == 515 && y == 506) // door
						return dett_grass;
					return dett_wall;
				}
			}
			return dett_grass;
		}
	}

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

void detmap_get_initial_position(const detmap* m, int* x, int* y)
{
	*x = DETMAP_DIMENSION / 2;
	*y = DETMAP_DIMENSION / 2;
	if(m->town) {
		*y -= 10;
	}
}


