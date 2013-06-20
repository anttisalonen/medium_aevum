#ifndef MA_DISCUSSION_H
#define MA_DISCUSSION_H

struct discussion;
typedef struct discussion discussion;

discussion* discussion_create(void);
void discussion_cleanup(discussion* d);

#endif

