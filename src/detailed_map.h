#ifndef MA_DETAILEDMAP_H
#define MA_DETAILEDMAP_H

#define DETMAP_DIMENSION 1024

#include "person.h"
#include "town.h"
#include "terrain.h"

struct detmap;
typedef struct detmap detmap;

detmap* detmap_create(terrain_type tt, const town* t);
void detmap_cleanup(detmap* m);
detterrain_type detmap_get_terrain_at(const detmap* m, int x, int y);
void detmap_get_initial_position(const detmap* m, int* x, int* y);
detmap_overlay detmap_get_overlay_at(const detmap* m, int x, int y);

int detmap_passable(const detmap* m, int x, int y);

#endif

