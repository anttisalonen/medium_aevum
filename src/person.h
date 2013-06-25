#ifndef MA_PERSON_H
#define MA_PERSON_H

#include "discussion.h"
#include "transaction.h"
#include "town.h"

struct person;
typedef struct person person;

person* person_create(const town* hometown);
void person_cleanup(person* p);
discussion* person_start_discussion(const person* p);

int person_handle_transaction(person* p, const transaction* t);

#endif

