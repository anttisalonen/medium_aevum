#ifndef MA_TERRAIN_H
#define MA_TERRAIN_H

typedef enum {
	tt_grass,
	tt_forest,
	tt_hills,
	tt_sea,
} terrain_type;

typedef enum {
	dett_grass,
	dett_tree,
	dett_rock,
	dett_water,
	dett_wall,
} detterrain_type;

typedef enum {
	detmap_overlay_none,
	detmap_overlay_wall,
} detmap_overlay;


#endif

