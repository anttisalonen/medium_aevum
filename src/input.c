#include <assert.h>
#include <stdlib.h>

#include <SDL.h>

#include "input.h"

struct input {
	player* player;
	graphics* graphics;
	worldtime* time;
};

input* input_create(player* p, graphics* g, worldtime* w)
{
	input* i;
	if(SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL)) {
		fprintf(stderr, "SDL_EnableKeyRepeat: %s\n", SDL_GetError());
		return NULL;
	}

	i = malloc(sizeof(input));
	if(!i)
		return NULL;

	i->player = p;
	i->graphics = g;
	i->time = w;
	return i;
}

static int handle_exit_key_event(input* i, SDLKey key, int* quitting)
{
	switch(key) {
		case SDLK_ESCAPE:
		case SDLK_q:
			*quitting = 1;
			return 0;

		default:
			return 0;
	}

	return 0;
}

static int handle_key_event(input* i, Uint8 type, SDLKey key, int* quitting, int* redraw)
{
	static const float cam_velocity = 1.0f;
	float cam_x = 0.0f;
	float cam_y = 0.0f;
	int player_x = 0;
	int player_y = 0;
	int zooming = 0;

	if(type != SDL_KEYDOWN)
		return 0;

	if(handle_exit_key_event(i, key, quitting)) {
		return 1;
	}

	if(*quitting)
		return 0;

	switch(key) {
		case SDLK_w: cam_y = -cam_velocity; break;
		case SDLK_s: cam_y =  cam_velocity; break;
		case SDLK_a: cam_x = -cam_velocity; break;
		case SDLK_d: cam_x =  cam_velocity; break;
		case SDLK_UP:    player_y = -1; break;
		case SDLK_DOWN:  player_y = 1; break;
		case SDLK_LEFT:  player_x = -1; break;
		case SDLK_RIGHT: player_x = 1; break;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
				 if(!player_dead(i->player))
					 zooming = 1;
				 break;

		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
				 if(!player_dead(i->player)) {
					 discussion* d = player_get_discussion(i->player);
					 if(d) {
						 discussion_give_answer(d, key - SDLK_1);
						 *redraw = 1;
					 }
				 }
				 break;

		default:
			return 0;
	}

	if(cam_x || cam_y) {
		graphics_move_camera(i->graphics, cam_x, cam_y);
		*redraw = 1;
	}
	if(player_x || player_y) {
		int cam_pos_x, cam_pos_y;
		int plr_pos_x, plr_pos_y;
		if(player_move(i->player, player_x, player_y)) {
			*redraw = 1;

			player_get_current_position(i->player, &plr_pos_x, &plr_pos_y);
			graphics_get_camera_position(i->graphics, &cam_pos_x, &cam_pos_y);
			if(abs(cam_pos_x - plr_pos_x) > 8 || abs(cam_pos_y - plr_pos_y) > 8) {
				graphics_set_camera_position(i->graphics, plr_pos_x, plr_pos_y);
			}
		}
	}

	if(zooming) {
		player_zoom(i->player);
		*redraw = 1;
	}

	return 0;
}

static int handle_video_resize(input* i, const SDL_Event* event)
{
	int w, h;
	w = event->resize.w;
	h = event->resize.h;
	if(graphics_resized(i->graphics, w, h))
		return 1;
	return 0;
}

int input_handle(input* i, int* quitting)
{
	assert(i);
	assert(quitting);
	SDL_Event event;
	*quitting = 0;

	int sleeping = player_try_sleep(i->player);
	if(sleeping) {
		while(1) {
			int ret = SDL_PollEvent(&event);
			if(!ret) {
				// no events
				return 0;
			} else {
				switch(event.type) {
					case SDL_KEYDOWN:
						ret = handle_exit_key_event(i, event.key.keysym.sym, quitting);
						if(ret)
							return 1;
						if(*quitting)
							return 0;
						break;

					case SDL_QUIT:
						*quitting = 1;
						return 0;

					case SDL_VIDEORESIZE:
						if(handle_video_resize(i, &event))
							return 1;
						break;

					default:
						break;
				}
			}
		}
	}

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
				if(handle_video_resize(i, &event))
					return 1;
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


