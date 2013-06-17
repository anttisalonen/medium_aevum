#include <assert.h>
#include <stdlib.h>

#include "worldtime.h"

struct worldtime {
	int time; // minutes
};

worldtime* worldtime_create(void)
{
	worldtime* w = malloc(sizeof(worldtime));
	assert(w);
	w->time = 6 * 60;
	return w;
}

void worldtime_cleanup(worldtime* w)
{
	free(w);
}

int worldtime_get(const worldtime* w)
{
	return w->time;
}

void worldtime_get_timeofday(const worldtime* w, int* hours, int* minutes)
{
	*minutes = w->time % 60;
	*hours = (w->time / 60) % 24;
}

void worldtime_advance(worldtime* w, int minutes)
{
	assert(minutes >= 0);
	w->time += minutes;
}



