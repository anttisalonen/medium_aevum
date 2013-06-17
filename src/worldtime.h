#ifndef MA_WORLDTIME_H
#define MA_WORLDTIME_H

struct worldtime;
typedef struct worldtime worldtime;

worldtime* worldtime_create(void);
void worldtime_cleanup(worldtime* w);
int worldtime_get(const worldtime* w);
void worldtime_get_timeofday(const worldtime* w, int* hours, int* minutes);
void worldtime_advance(worldtime* w, int minutes);

#endif

