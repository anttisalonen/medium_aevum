#include <stdio.h>
#include <unistd.h>

#include "graphics.h"

int run_game(void)
{
	int width = 800;
	int height = 600;
	int ret;
	map* m = map_create();
	graphics* gr = graphics_init(width, height, m);
	if(!gr) {
		map_cleanup(m);
		return 1;
	}
	ret = graphics_draw(gr);
	sleep(2);
	graphics_cleanup(gr);
	map_cleanup(m);
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
