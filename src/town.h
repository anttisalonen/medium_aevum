#ifndef MA_TOWN_H
#define MA_TOWN_H

struct town;
typedef struct town town;

town* town_create();
void town_cleanup(town* t);

#endif

