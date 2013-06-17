#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "mapinfo.h"

struct map {
	int time; // minutes

	int player_x;
	int player_y;

	unsigned char player_hunger; // 255 => starved
	unsigned char player_food;   // in lbs
};

map* map_create()
{
	map* m = malloc(sizeof(map));
	assert(m);

	memset(m, 0x00, sizeof(*m));

	m->player_x = 10000;
	m->player_y = 20000;
	m->player_food = 10;

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

static void map_handle_movement(map* m)
{
	m->time += 10 + rand() % 5;
	if(m->player_hunger < 255)
		m->player_hunger += rand() % 2;

	if(m->player_hunger > 40 && m->player_food) {
		m->player_food--;
		m->player_hunger -= rand() % 10 + 30;
	}
}

void map_move_player(map* m, int x, int y)
{
	terrain_type tt;

	if(m->player_hunger == 255)
		return;

	m->player_x += x;
	m->player_y += y;

	tt = map_get_terrain_at(m, m->player_x, m->player_y);
	if(tt == tt_sea) {
		m->player_x -= x;
		m->player_y -= y;
	} else {
		map_handle_movement(m);
	}
}

int map_get_time(const map* m)
{
	return m->time;
}

void map_get_timeofday(const map* m, int* hours, int* minutes)
{
	*minutes = m->time % 60;
	*hours = (m->time / 60 + 12) % 24;
}

unsigned char map_get_player_hunger(const map* m)
{
	return m->player_hunger;
}


