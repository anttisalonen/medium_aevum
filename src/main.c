#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "graphics.h"
#include "input.h"
#include "worldtime.h"

static int run_game(void)
{
	int width = 800;
	int height = 600;
	int ret;
	map* m = map_create();
	worldtime* w = worldtime_create();
	player* p = player_create(m, w);
	graphics* gr = graphics_create(width, height, p, w);
	if(!gr) {
		player_cleanup(p);
		map_cleanup(m);
		worldtime_cleanup(w);
		return 1;
	}
	input* input = input_create(p, gr, w);
	if(!input) {
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
		ret = input_handle(input, &quitting);
		if(ret)
			break;
		ret = graphics_draw(gr);
		if(ret)
			break;
	}

	input_cleanup(input);
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
