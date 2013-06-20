#ifndef MA_DISCUSSION_H
#define MA_DISCUSSION_H

struct discussion;
typedef struct discussion discussion;

discussion* discussion_create(void);
void discussion_cleanup(discussion* d);

const char* discussion_get_line(discussion* d);
int discussion_get_answers(const discussion* d, char*** answers);
void discussion_give_answer(discussion* d, int n);

#endif

