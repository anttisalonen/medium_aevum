#ifndef MA_PERSON_H
#define MA_PERSON_H

#include "discussion.h"

struct person;
typedef struct person person;

person* person_create(void);
void person_cleanup(person* p);
discussion* person_start_discussion(const person* p);

#endif

