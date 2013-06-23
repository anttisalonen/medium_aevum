#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "random.h"
#include "detailed_map.h"

struct detmap {
	terrain_type terrain;
	const town* town;
	detterrain_type terrmap[DETMAP_DIMENSION][DETMAP_DIMENSION];
	detmap_overlay overlaymap[DETMAP_DIMENSION][DETMAP_DIMENSION];
};

static void build_terrain_map(detmap* m)
{
	for(int j = 0; j < DETMAP_DIMENSION; j++) {
		for(int i = 0; i < DETMAP_DIMENSION; i++) {
			int r = my_rand() % 20;
			if(r == 0)
				m->terrmap[i][j] = dett_water;
			else if(r <= 2)
				m->terrmap[i][j] = dett_rock;
			else if(r <= 4)
				m->terrmap[i][j] = dett_tree;
			else
				m->terrmap[i][j] = dett_grass;
		}
	}
}

static void build_overlay_map(detmap* m)
{
	if(m->town) {
		int num_houses = my_rand() % 5 + 10;
		for(int n = 0; n < num_houses; n++) {
			const int width = my_rand() % 10 + 4;
			const int height = my_rand() % 10 + 4;
			const int x = my_rand() % DETMAP_DIMENSION;
			const int y = my_rand() % DETMAP_DIMENSION;

			if(x + width - 1 >= DETMAP_DIMENSION)
				continue;
			if(y + height - 1 >= DETMAP_DIMENSION)
				continue;

			int has_already_house = 0;
			for(int yy = -1; yy <= height + 1; yy++) {
				for(int xx = -1; xx <= width + 1; xx++) {
					int xp = x + xx;
					int yp = y + yy;
					if(m->overlaymap[xp][yp] == detmap_overlay_wall) {
						has_already_house = 1;
						break;
					}
					if(xp >= 0 && xp < DETMAP_DIMENSION && yp >= 0 && yp < DETMAP_DIMENSION)
						m->terrmap[xp][yp] = dett_grass;
				}
				if(has_already_house)
					break;
			}
			if(has_already_house)
				continue;

			const int door_at = my_rand() % 4;
			for(int yy = 0; yy <= height; yy++) {
				if(door_at != 0 || yy != 3)
					m->overlaymap[x][y + yy] = detmap_overlay_wall;
			}
			for(int yy = 0; yy <= height; yy++) {
				if(door_at != 1 || yy != 3)
					m->overlaymap[x + width][y + yy] = detmap_overlay_wall;
			}
			for(int xx = 0; xx <= width; xx++) {
				if(door_at != 2 || xx != 3)
					m->overlaymap[x + xx][y] = detmap_overlay_wall;
			}
			for(int xx = 0; xx <= width; xx++) {
				if(door_at != 3 || xx != 3)
					m->overlaymap[x + xx][y + height] = detmap_overlay_wall;
			}
		}
	}
}

detmap* detmap_create(terrain_type tt, const town* t, int x, int y, int* persons_x, int* persons_y, int* num_persons)
{
	detmap* m = malloc(sizeof(detmap));
	assert(m);

	memset(m, 0x00, sizeof(*m));
	m->terrain = tt;
	m->town = t;

	my_rand_push((x & 0xffffffff) << 16 | y);
	build_terrain_map(m);
	build_overlay_map(m);
	*num_persons = detmap_get_initial_npc_positions(m, persons_x, persons_y, *num_persons);
	my_rand_pop();

	return m;
}

void detmap_cleanup(detmap* m)
{
	free(m);
}

detterrain_type detmap_get_terrain_at(const detmap* m, int x, int y)
{
	if(x < 0 || y < 0 || x >= DETMAP_DIMENSION || y >= DETMAP_DIMENSION)
		return dett_grass;

	return m->terrmap[x][y];
}

detmap_overlay detmap_get_overlay_at(const detmap* m, int x, int y)
{
	if(x < 0 || y < 0 || x >= DETMAP_DIMENSION || y >= DETMAP_DIMENSION)
		return detmap_overlay_none;

	return m->overlaymap[x][y];
}

static int suitable_initial_position(const detmap* m, int x, int y)
{
	int passing_around = 0;
	if(!detmap_passable(m, x, y))
		return 0;

	for(int j = -1; j <= 1; j++) {
		for(int i = -1; i <= 1; i++) {
			if(i || j)
				if(detmap_passable(m, x + i, y + j))
					passing_around++;
		}
	}
	return passing_around > 3;
}

void detmap_get_initial_position(const detmap* m, int* x, int* y)
{
	*x = DETMAP_DIMENSION / 2;
	*y = DETMAP_DIMENSION / 2;
	if(m->town) {
		*y -= 10;
	}

	if(!suitable_initial_position(m, *x, *y)) {
		if(suitable_initial_position(m, *x + 1, *y))
			(*x)++;
		else if(suitable_initial_position(m, *x - 1, *y))
			(*x)--;
		else if(suitable_initial_position(m, *x, *y + 1))
			(*y)++;
		else if(suitable_initial_position(m, *x, *y - 1))
			(*y)--;
	}
}

void detmap_get_initial_npc_position(detmap* m, int* x, int* y)
{
	for(int n = 100; n >= 0; n--) {
		*x = my_rand() % (DETMAP_DIMENSION - 10) - 5;
		*y = my_rand() % (DETMAP_DIMENSION - 10) - 5;
		if(!detmap_passable(m, *x, *y))
			continue;
		for(int j = -1; j <= 1; j++) {
			for(int i = -1; i <= 1; i++) {
				if(detmap_get_overlay_at(m, *x + i, *y + j) == detmap_overlay_wall)
					return;
			}
		}
	}
}

int detmap_get_initial_npc_positions(detmap* m, int* x, int* y, int bufsiz)
{
	if(!m->town)
		return 0;

	for(int i = 0; i < bufsiz; i++) {
		detmap_get_initial_npc_position(m, &x[i], &y[i]);
	}
	return bufsiz;
}

int detmap_passable(const detmap* m, int x, int y)
{
	detmap_overlay overlay = detmap_get_overlay_at(m, x, y);
	switch(overlay) {
		case detmap_overlay_none:
			break;

		case detmap_overlay_wall:
			return 0;
	}

	detterrain_type dt = detmap_get_terrain_at(m, x, y);
	if(dt != dett_grass ||
			x >= DETMAP_DIMENSION ||
			y >= DETMAP_DIMENSION ||
			x < 0 ||
			y < 0) {
		return 0;
	}

	return 1;
}
