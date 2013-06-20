#ifndef MA_DISCUSSION_H
#define MA_DISCUSSION_H

#include "transaction.h"

struct discussion;
typedef struct discussion discussion;

discussion* discussion_create(int can_give_food);
void discussion_cleanup(discussion* d);

const char* discussion_get_line(discussion* d);
int discussion_get_answers(const discussion* d, char*** answers);
transaction discussion_give_answer(discussion* d, int n);

#endif

