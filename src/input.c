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
	input* i;
	if(SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL)) {
		fprintf(stderr, "SDL_EnableKeyRepeat: %s\n", SDL_GetError());
		return NULL;
	}

	i = malloc(sizeof(input));
	if(!i)
		return NULL;

	i->map = m;
	i->graphics = g;
	return i;
}

static int handle_key_event(input* i, Uint8 type, SDLKey key, int* quitting, int* redraw)
{
	static const float cam_velocity = 0.1f;
	float cam_x = 0.0f;
	float cam_y = 0.0f;

	switch(key) {
		case SDLK_ESCAPE:
		case SDLK_q:
			*quitting = 1;
			return 0;

		case SDLK_w:
		case SDLK_a:
		case SDLK_s:
		case SDLK_d:
			if(type == SDL_KEYDOWN) {
				if(key == SDLK_w)
					cam_y = -cam_velocity;
				else if(key == SDLK_a)
					cam_x = -cam_velocity;
				else if(key == SDLK_s)
					cam_y = cam_velocity;
				else
					cam_x = cam_velocity;
				graphics_move_camera(i->graphics, cam_x, cam_y);
				*redraw = 1;
				return 0;
			}
			return 0;

		default:
			return 0;
	}

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
				ret = handle_key_event(i, event.key.type, event.key.keysym.sym, quitting, &redraw);
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


