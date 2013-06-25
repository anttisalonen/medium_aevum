#ifndef MA_TOWN_H
#define MA_TOWN_H

struct town;
typedef struct town town;

town* town_create(const char* name, int x, int y);
void town_cleanup(town* t);
void town_get_location(const town* t, int* x, int* y);
const char* town_get_name(const town* t);

#endif

