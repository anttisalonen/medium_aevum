#ifndef MA_DETAILEDMAP_H
#define MA_DETAILEDMAP_H

#define DETMAP_DIMENSION 128

#include "person.h"
#include "town.h"
#include "terrain.h"

struct detmap;
typedef struct detmap detmap;

detmap* detmap_create(terrain_type tt, const town* t, int x, int y, int* persons_x, int* persons_y, int* num_persons);
void detmap_cleanup(detmap* m);
detterrain_type detmap_get_terrain_at(const detmap* m, int x, int y);
void detmap_get_initial_position(const detmap* m, int* x, int* y);
void detmap_get_initial_npc_position(detmap* m, int* x, int* y);
int detmap_get_initial_npc_positions(detmap* m, int* x, int* y, int bufsiz);
detmap_overlay detmap_get_overlay_at(const detmap* m, int x, int y);

int detmap_passable(const detmap* m, int x, int y);

#endif

