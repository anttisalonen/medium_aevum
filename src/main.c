#include <stdio.h>
#include <SDL.h>

int init_sdl(void)
{
	int err;
	const SDL_VideoInfo* info;
	SDL_Surface* surface;

	err = SDL_Init(SDL_INIT_VIDEO);
	if(err) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

	info = SDL_GetVideoInfo();
	if(!info) {
		fprintf(stderr, "SDL_GetVideoInfo: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	surface = SDL_SetVideoMode(800, 600, info->vfmt->BitsPerPixel, SDL_OPENGL);
	if(!surface) {
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		return 1;
	}

	return 0;
}

int main(void)
{
	int ret = init_sdl();
	if(ret) {
		return 1;
	} else {
		return 0;
	}
}
