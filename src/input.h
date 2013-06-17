#ifndef MA_INPUT_H
#define MA_INPUT_H

#include "graphics.h"
#include "player.h"
#include "worldtime.h"

struct input;
typedef struct input input;

input* input_create(player* p, graphics* g, worldtime* w);
int input_handle(input* i, int* quitting);
void input_cleanup(input* i);

#endif

