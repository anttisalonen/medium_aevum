#ifndef MA_PLAYER_H
#define MA_PLAYER_H

#include "mapinfo.h"
#include "worldtime.h"
#include "detailed_map.h"
#include "person_directory.h"
#include "person.h"
#include "discussion.h"

struct player;
typedef struct player player;

player* player_create(map* m, worldtime* w, person_directory* pd);
void player_cleanup(player* p);

void player_get_position(const player* p, int* x, int* y);
int player_get_detmap_position(const player* p, int* dx, int* dy);
// get_current_position returns coordinates in whatever detail leve
// the player currently is in
void player_get_current_position(const player* p, int* nx, int* ny);

detmap* player_get_detmap(player* p);
map* player_get_map(player* p);
int player_move(player* p, int x, int y);

unsigned char player_get_hunger(const player* p);
int player_dead(const player* p);

int player_try_sleep(player* p);
int player_sleeping(const player* p);

int player_zoom(player* p);

discussion* player_get_discussion(player* p);

#endif

