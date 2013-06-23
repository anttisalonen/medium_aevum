#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "graphics.h"
#include "input.h"
#include "worldtime.h"
#include "person.h"
#include "person_directory.h"

static int run_game(void)
{
	int width = 800;
	int height = 600;
	int ret;
	map* m = map_create();
	worldtime* w = worldtime_create();
	person* npc = person_create();

	person_directory* pd = person_directory_create();
	assert(pd);
	assert(npc);
	person_directory_add_person(pd, 10003, 20005, 513, 509, npc);
	player* p = player_create(m, w, pd);

	graphics* gr = graphics_create(width, height, p, w, pd);
	if(!gr) {
		player_cleanup(p);
		map_cleanup(m);
		worldtime_cleanup(w);
		return 1;
	}
	input* inp = input_create(p, gr, w);
	if(!inp) {
		graphics_cleanup(gr);
		player_cleanup(p);
		map_cleanup(m);
		worldtime_cleanup(w);
		return 1;
	}

	srand(21);

	int quitting = 0;
	ret = graphics_draw(gr);
	if(ret)
		quitting = 1;

	while(!quitting) {
		ret = input_handle(inp, &quitting);
		if(ret)
			break;
		ret = graphics_draw(gr);
		if(ret)
			break;
	}

	input_cleanup(inp);
	graphics_cleanup(gr);
	map_cleanup(m);
	player_cleanup(p);
	worldtime_cleanup(w);
	return ret;
}

int main(void)
{
	int ret = run_game();
	if(ret) {
		return 1;
	} else {
		return 0;
	}
}
