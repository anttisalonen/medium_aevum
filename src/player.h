#ifndef MA_PLAYER_H
#define MA_PLAYER_H

#include "mapinfo.h"
#include "worldtime.h"

struct player;
typedef struct player player;

player* player_create(map* m, worldtime* w);
void player_cleanup(player* p);
void player_get_position(const player* p, int* x, int* y);
map* player_get_map(player* p);
int player_move(player* p, int x, int y);

unsigned char player_get_hunger(const player* p);

int player_try_sleep(player* p);
int player_sleeping(const player* p);

#endif

