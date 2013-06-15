#ifndef MA_GRAPHICS_H
#define MA_GRAPHICS_H

struct graphics;
typedef struct graphics graphics;

graphics* graphics_init(int width, int height);
int graphics_draw(graphics* g);
void graphics_cleanup(graphics* g);

#endif

