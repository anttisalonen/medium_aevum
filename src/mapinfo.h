#ifndef MA_MAPINFO_H
#define MA_MAPINFO_H

struct map;
typedef struct map map;

typedef enum {
	tt_grass,
	tt_forest,
	tt_hills,
	tt_sea,
} terrain_type;

map* map_create();
void map_cleanup(map* m);
terrain_type map_get_terrain_at(const map* m, int x, int y);
void map_get_player_position(const map* m, int* x, int* y);
void map_move_player(map* m, int x, int y);
int map_get_time(const map* m);
void map_get_timeofday(const map* m, int* hours, int* minutes);

#endif

