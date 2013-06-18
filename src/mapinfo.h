#ifndef MA_MAPINFO_H
#define MA_MAPINFO_H

#include "terrain.h"
#include "town.h"

struct map;
typedef struct map map;

map* map_create();
void map_cleanup(map* m);
terrain_type map_get_terrain_at(const map* m, int x, int y);
const town* map_get_town_at(const map* m, int x, int y);

#endif

