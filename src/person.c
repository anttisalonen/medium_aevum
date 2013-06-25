#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "person.h"

struct person {
	const town* hometown;
	int have_food_to_give_away;
};

person* person_create(const town* hometown)
{
	person* p = malloc(sizeof(person));
	assert(p);

	memset(p, 0x00, sizeof(*p));
	p->hometown = hometown;

	p->have_food_to_give_away = 1;

	return p;
}

void person_cleanup(person* p)
{
	free(p);
}

discussion* person_start_discussion(const person* p)
{
	return discussion_create(p->hometown, p->have_food_to_give_away);
}

int person_handle_transaction(person* p, const transaction* t)
{
	switch(t->type) {
		case transaction_none:
			return 0;

		case transaction_give_food:
			assert(p->have_food_to_give_away);
			p->have_food_to_give_away = 0;
			return 0;
	}

	assert(0);
	return 1;
}


