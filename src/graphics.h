#ifndef MA_GRAPHICS_H
#define MA_GRAPHICS_H

#include "player.h"
#include "worldtime.h"
#include "person_directory.h"
#include "discussion.h"

struct graphics;
typedef struct graphics graphics;

graphics* graphics_create(int width, int height, player* p, worldtime* w, person_directory* pd);
void graphics_cleanup(graphics* g);

int graphics_draw(graphics* g);
int graphics_resized(graphics* g, int w, int h);
void graphics_move_camera(graphics* g, float x, float y);
void graphics_get_camera_position(const graphics* g, int* x, int* y);
void graphics_set_camera_position(graphics* g, int x, int y);

#endif

