#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "person.h"

struct person {
	int x;
};

person* person_create(void)
{
	person* p = malloc(sizeof(person));
	assert(p);

	memset(p, 0x00, sizeof(*p));

	return p;
}

void person_cleanup(person* p)
{
	free(p);
}

