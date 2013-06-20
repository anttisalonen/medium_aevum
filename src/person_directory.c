#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "person_directory.h"

struct person_directory {
	int mx;
	int my;
	int dx;
	int dy;
	person* person;
};

person_directory* person_directory_create(void)
{
	person_directory* pd = malloc(sizeof(person_directory));
	assert(pd);

	memset(pd, 0x00, sizeof(*pd));

	return pd;
}

void person_directory_cleanup(person_directory* pd)
{
	free(pd);
}

const person* person_directory_get_person_at(const person_directory* pd, int mx, int my, int dx, int dy)
{
	if(pd->mx == mx && pd->my == my && pd->dx == dx && pd->dy == dy)
		return pd->person;

	return NULL;
}

int person_directory_get_num_people(const person_directory* pd, int mx, int my)
{
	if(pd->mx == mx && pd->my == my)
		return 1;

	return 0;
}

int person_directory_get_people(const person_directory* pd, int mx, int my, const person** people, unsigned int bufsiz)
{
	if(person_directory_get_num_people(pd, mx, my) == 0)
		return 0;

	if(bufsiz < 1)
		return 0;

	people[0] = pd->person;
	return 1;
}

void person_directory_get_person_position(const person_directory* pd, const person* person, int* dx, int* dy)
{
	assert(person == pd->person);
	*dx = pd->dx;
	*dy = pd->dy;
}

void person_directory_add_person(person_directory* pd, int mx, int my, int dx, int dy, person* person)
{
	assert(!pd->person);
	pd->person = person;
	pd->mx = mx;
	pd->my = my;
	pd->dx = dx;
	pd->dy = dy;
}

void person_directory_act(person_directory* pd, int mx, int my, const detmap* detm)
{
	if(pd->mx == mx && pd->my == my) {
		if(pd->person) {
			int want_move = rand() % 4 == 0;
			if(want_move) {
				int nx = rand() % 3 - 1;
				int ny = rand() % 3 - 1;
				if(nx || ny) {
					int new_x = pd->dx + nx;
					int new_y = pd->dy + ny;
					if(detmap_passable(detm, new_x, new_y) &&
							!person_directory_get_person_at(pd, mx, my, new_x, new_y)) {
						pd->dx = new_x;
						pd->dy = new_y;
					}
				}
			}
		}
	}
}


