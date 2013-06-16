#ifndef MA_GRAPHICS_H
#define MA_GRAPHICS_H

#include "mapinfo.h"

struct graphics;
typedef struct graphics graphics;

graphics* graphics_init(int width, int height, map* m);
void graphics_cleanup(graphics* g);

int graphics_draw(graphics* g);
int graphics_resized(graphics* g, int w, int h);
void graphics_move_camera(graphics* g, float x, float y);
void graphics_get_camera_position(const graphics* g, int* x, int* y);
void graphics_set_camera_position(graphics* g, int x, int y);

#endif

