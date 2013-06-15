#ifndef MA_INPUT_H
#define MA_INPUT_H

#include "graphics.h"
#include "mapinfo.h"

struct input;
typedef struct input input;

input* input_init(map* m, graphics* g);
int input_handle(input* i, int* quitting);
void input_cleanup(input* i);

#endif

