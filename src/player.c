#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "player.h"

struct player {
	int x;
	int y;

	map* map;
	detmap* detmap;
	worldtime* time;
	person_directory* pd;

	unsigned char hunger;   // 255 => starved
	unsigned char food;     // in lbs
	unsigned char fatigue;  // > 240 => can sleep
	unsigned char to_sleep; // == 0 => awake

	int d_x;
	int d_y;
};

player* player_create(map* m, worldtime* w, person_directory* pd)
{
	player* p = malloc(sizeof(player));
	assert(p);
	memset(p, 0x00, sizeof(*p));

	p->map = m;
	p->time = w;
	p->pd = pd;
	p->x = 10000;
	p->y = 20000;
	p->food = 10;
	p->fatigue = 0;

	return p;
}

void player_cleanup(player* p)
{
	free(p);
}

void player_get_position(const player* p, int* x, int* y)
{
	*x = p->x;
	*y = p->y;
}

int player_get_detmap_position(const player* p, int* dx, int* dy)
{
	if(!p->detmap) {
		return 0;
	} else {
		*dx = p->d_x;
		*dy = p->d_y;
		return 1;
	}
}

void player_get_current_position(const player* p, int* nx, int* ny)
{
	if(p->detmap)
		player_get_detmap_position(p, nx, ny);
	else
		player_get_position(p, nx, ny);
}

map* player_get_map(player* p)
{
	return p->map;
}

detmap* player_get_detmap(player* p)
{
	return p->detmap;
}

int occasion(int every_n_minutes, int minutes_passed)
{
	assert(every_n_minutes);
	int ret = minutes_passed / every_n_minutes;

	int rest = minutes_passed % every_n_minutes;
	if(rest) {
		if(rand() % every_n_minutes < minutes_passed - ret * every_n_minutes)
			ret++;
	}
	return ret;
}

void bounded_increment(unsigned char* value, unsigned char inc)
{
	if(255 - *value < inc)
		*value = 255;
	else
		*value += inc;
}

int player_time_advanced(player* p, int minutes)
{
	int more_hunger = occasion(24, minutes);
	bounded_increment(&p->hunger, more_hunger);

	if(!player_sleeping(p)) {
		int more_fatigue = occasion(42, minutes * 10);
		bounded_increment(&p->fatigue, more_fatigue);
		if(p->hunger > 40 && p->food) {
			p->food--;
			p->hunger -= rand() % 10 + 30;
		}
	}

	return 0;
}

static void player_handle_movement(player* p)
{
	int t = 10 + rand() % 5;
	worldtime_advance(p->time, t);
	player_time_advanced(p, t);
}

static void player_handle_detmap_movement(player* p)
{
	int t = rand() % 6 == 0;
	if(t) {
		worldtime_advance(p->time, t);
		player_time_advanced(p, t);
	}
}

int player_dead(const player* p)
{
	return p->hunger == 255 || p->fatigue == 255;
}

int player_move(player* p, int x, int y)
{
	if(player_dead(p))
		return 0;

	if(p->detmap) {
		p->d_x += x;
		p->d_y += y;

		if(!detmap_passable(p->detmap, p->d_x, p->d_y) || person_directory_get_person_at(p->pd, p->x, p->y, p->d_x, p->d_y)) {
			p->d_x -= x;
			p->d_y -= y;
			return 0;
		} else {
			player_handle_detmap_movement(p);
			return 1;
		}
	} else {
		terrain_type tt;

		p->x += x;
		p->y += y;

		tt = map_get_terrain_at(p->map, p->x, p->y);
		if(tt == tt_sea) {
			p->x -= x;
			p->y -= y;
			return 0;
		} else {
			player_handle_movement(p);
			return 1;
		}
	}
}

unsigned char player_get_hunger(const player* p)
{
	return p->hunger;
}

int player_try_sleep(player* p)
{
	if(p->to_sleep == 0) {
		if(p->fatigue > 240) {
			p->to_sleep = p->fatigue;
		}
	}

	if(p->to_sleep > 0) {
		static const int minutes = 10;
		worldtime_advance(p->time, minutes);
		player_time_advanced(p, minutes);
		for(int i = 0; i < minutes; i++) {
			int from_fatigue = rand() % 2 == 0;

			// 240 * 2 minutes ~= 8 hours of sleep
			if(!from_fatigue) {
				if(p->to_sleep) {
					p->to_sleep--;
				} else {
					p->to_sleep = 0;
				}
			} else {
				if(p->fatigue) {
					p->fatigue--;
				} else {
					p->fatigue = 0;
				}
			}
		}
		return 1;
	}
	return 0;
}

int player_sleeping(const player* p)
{
	return p->to_sleep > 0;
}

int player_zoom(player* p)
{
	if(p->detmap) {
		detmap_cleanup(p->detmap);
		p->detmap = NULL;
	} else {
		terrain_type tt = map_get_terrain_at(p->map, p->x, p->y);
		const town* t = map_get_town_at(p->map, p->x, p->y);
		p->detmap = detmap_create(tt, t);
		detmap_get_initial_position(p->detmap, &p->d_x, &p->d_y);
	}
	return 0;
}


