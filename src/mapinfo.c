#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mapinfo.h"

struct map {
	int player_x;
	int player_y;
};

map* map_create()
{
	map* m = malloc(sizeof(map));
	assert(m);
	m->player_x = 10000;
	m->player_y = 20000;
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

void map_get_player_position(const map* m, int* x, int* y)
{
	*x = m->player_x;
	*y = m->player_y;
}

void map_move_player(map* m, int x, int y)
{
	m->player_x += x;
	m->player_y += y;
}


