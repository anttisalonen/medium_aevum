#include <assert.h>
#include <stdlib.h>

#include <SDL.h>

#include "input.h"

struct input {
	map* map;
	graphics* graphics;
};

input* input_init(map* m, graphics* g)
{
	input* i = malloc(sizeof(input));
	if(!i)
		return NULL;

	i->map = m;
	i->graphics = g;
	return i;
}

static int handle_key_event(Uint8 type, SDLKey key, int* quitting, int* redraw)
{
	if(key == SDLK_q)
		*quitting = 1;

	if(type == SDL_KEYDOWN && (key == SDLK_KP_ENTER || key == SDLK_RETURN))
		*redraw = 1;

	return 0;
}

int input_handle(input* i, int* quitting)
{
	assert(i);
	assert(quitting);
	SDL_Event event;
	*quitting = 0;
	while(1) {
		int ret = SDL_WaitEvent(&event);
		int redraw = 0;
		if(!ret)
			return 1;

		switch(event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				ret = handle_key_event(event.key.type, event.key.keysym.sym, quitting, &redraw);
				if(ret)
					return 1;
				break;

			case SDL_QUIT:
				*quitting = 1;
				break;

			default:
				break;
		}

		if(redraw || *quitting)
			break;
	}

	return 0;
}

void input_cleanup(input* i)
{
	free(i);
}


