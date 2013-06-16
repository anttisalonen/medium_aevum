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
	static const float cam_velocity = 1.0f;
	float cam_x = 0.0f;
	float cam_y = 0.0f;
	int player_x = 0;
	int player_y = 0;

	if(type != SDL_KEYDOWN)
		return 0;

	switch(key) {
		case SDLK_ESCAPE:
		case SDLK_q:
			*quitting = 1;
			return 0;

		case SDLK_w: cam_y = -cam_velocity; break;
		case SDLK_s: cam_y =  cam_velocity; break;
		case SDLK_a: cam_x = -cam_velocity; break;
		case SDLK_d: cam_x =  cam_velocity; break;
		case SDLK_UP:    player_y = -1; break;
		case SDLK_DOWN:  player_y = 1; break;
		case SDLK_LEFT:  player_x = -1; break;
		case SDLK_RIGHT: player_x = 1; break;

		default:
			return 0;
	}

	if(cam_x || cam_y) {
		graphics_move_camera(i->graphics, cam_x, cam_y);
		*redraw = 1;
	}
	if(player_x || player_y) {
		map_move_player(i->map, player_x, player_y);
		*redraw = 1;
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

			case SDL_VIDEORESIZE:
				{
					int w, h;
					w = event.resize.w;
					h = event.resize.h;
					if(graphics_resized(i->graphics, w, h))
						return 1;
				}
				redraw = 1;
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


