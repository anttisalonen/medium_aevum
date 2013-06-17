#ifndef MA_DETAILEDMAP_H
#define MA_DETAILEDMAP_H

#define DETMAP_DIMENSION 1024

struct detmap;
typedef struct detmap detmap;

typedef enum {
	dett_grass,
	dett_tree,
	dett_rock,
	dett_water,
} detterrain_type;

detmap* detmap_create();
void detmap_cleanup(detmap* m);
detterrain_type detmap_get_terrain_at(const detmap* m, int x, int y);

#endif

