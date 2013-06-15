#ifndef MA_GRAPHICS_H
#define MA_GRAPHICS_H

#include "mapinfo.h"

struct graphics;
typedef struct graphics graphics;

graphics* graphics_init(int width, int height, map* m);
int graphics_draw(graphics* g);
void graphics_move_camera(graphics* g, float x, float y);
void graphics_cleanup(graphics* g);

#endif

