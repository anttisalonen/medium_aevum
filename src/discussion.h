#ifndef MA_DISCUSSION_H
#define MA_DISCUSSION_H

#include "transaction.h"
#include "town.h"

struct discussion;
typedef struct discussion discussion;

discussion* discussion_create(const town* t, int can_give_food);
void discussion_cleanup(discussion* d);

const char* discussion_get_line(discussion* d);
int discussion_get_answers(const discussion* d, char*** answers);
transaction discussion_give_answer(discussion* d, int n);

#endif

