#include <stdio.h>
#include "graphics.h"

int run_game(void)
{
	int width = 800;
	int height = 600;
	int ret;
	graphics* gr = graphics_init(width, height);
	if(!gr)
		return 1;
	ret = graphics_draw(gr);
	graphics_cleanup(gr);
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
