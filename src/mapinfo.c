#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>

#include "xmalloc.h"
#include "mapinfo.h"

struct townlist;

struct map {
	terrain_type* terrain_data;
	unsigned short terrain_x;
	unsigned short terrain_y;
	town** towns;
	int num_towns;
};

static float load_terrain_data(map* m)
{
	SDL_Surface* surf = IMG_Load("share/europe.png");
	if(!surf) {
		fprintf(stderr, "IMG_Load error on europe.png: %s\n", IMG_GetError());
		abort();
	}
	if(surf->w != 1110 || surf->h != 568) {
		fprintf(stderr, "europe.png must be 1110x568.\n");
		abort();
	}
	SDL_PixelFormat* fmt = surf->format;
	m->terrain_x = surf->w;
	m->terrain_y = surf->h;

	float diagonal = sqrt(m->terrain_x * m->terrain_x + m->terrain_y * m->terrain_y);
	assert(diagonal);
	const float scaling = 6151.0f / diagonal;
	m->terrain_x *= scaling;
	m->terrain_y *= scaling;

	printf("Map with size %dx%d - scaling %3.2f.\n", m->terrain_x, m->terrain_y, scaling);

	m->terrain_data = xmalloc(sizeof(terrain_type) * m->terrain_x * m->terrain_y);

	if(surf->format->BytesPerPixel != 4) {
		fprintf(stderr, "europe.png must be 4 BPP.\n");
		abort();
	}

	for(int j = 0; j < m->terrain_y; j++) {
		for(int i = 0; i < m->terrain_x; i++) {
			int px = i / scaling;
			int py = j / scaling;
			assert(px < surf->w);
			assert(py < surf->h);
			Uint32 pixel = ((Uint32*)surf->pixels)[py * surf->w + px];
			Uint8 red, green, blue;
			red   = (pixel & fmt->Rmask) >> fmt->Rshift;
			green = (pixel & fmt->Gmask) >> fmt->Gshift;
			blue  = (pixel & fmt->Bmask) >> fmt->Bshift;
			if(blue > green && blue > red) {
				m->terrain_data[j * m->terrain_x + i] = tt_sea;
			} else {
				m->terrain_data[j * m->terrain_x + i] = tt_grass;
			}
		}
	}
	SDL_FreeSurface(surf);
	return scaling;
}

void load_town_data(map* m, float scaling)
{
	FILE* f = fopen("share/townlist.txt", "r");
	if(!f) {
		fprintf(stderr, "Can't load towns\n");
		assert(0);
		return;
	}

	int linenum = 1;

	while(1) {
		char buf[1024];
		memset(buf, 0x00, sizeof(buf));
		char* ret = fgets(buf, 1024, f);
		if(!ret)
			break;
		char townname[256];
		memset(townname, 0x00, sizeof(townname));
		float xcoord, ycoord;
		char* colon = strchr(buf, ':');
		if(!colon) {
			fprintf(stderr, "townlist: line %d: no colon\n", linenum);
			assert(0);
			return;
		}
		*colon = 0;
		strncpy(townname, buf, 256);
		int scanret = sscanf(colon + 1, "%f,%f", &xcoord, &ycoord);
		if(scanret != 2) {
			fprintf(stderr, "townlist: line %d: no coordinates\n", linenum);
			assert(0);
			return;
		}

		{
			int townx = 15.36f * xcoord + 382;
			int towny = (-15.0f) * ycoord + 995;
			townx *= scaling;
			towny *= scaling;

			m->num_towns++;
			town* t = town_create(townname, townx, towny);
			void* realloc_ret = realloc(m->towns, m->num_towns * sizeof(town*));
			assert(realloc_ret);
			m->towns = realloc_ret;
			m->towns[m->num_towns - 1] = t;

			linenum++;
		}
	}

	fclose(f);
}

map* map_create(void)
{
	map* m = xmalloc(sizeof(map));

	float scaling = load_terrain_data(m);
	load_town_data(m, scaling);

	return m;
}

void map_cleanup(map* m)
{
	for(int i = 0; i < m->num_towns; i++)
		town_cleanup(m->towns[i]);
	xfree(m->towns);
	xfree(m->terrain_data);
	xfree(m);
}

terrain_type map_get_terrain_at(const map* m, int x, int y)
{
	if(x < 0 || y < 0 || x >= m->terrain_x || y >= m->terrain_y) {
		return tt_sea;
	} else {
		return m->terrain_data[y * m->terrain_x + x];
	}
}

const town* map_get_town_at(const map* m, int x, int y)
{
	for(int i = 0; i < m->num_towns; i++) {
		int tx, ty;
		town_get_location(m->towns[i], &tx, &ty);
		if(tx == x && ty == y)
			return m->towns[i];
	}
	return NULL;
}


