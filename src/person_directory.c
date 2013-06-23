#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "random.h"
#include "xmalloc.h"

#include "person_directory.h"

typedef struct {
	int mx;
	int my;
	int dx;
	int dy;
	person* person;
} person_record;

#define MAX_NUM_PERSONS 32

struct person_directory {
	person_record records[MAX_NUM_PERSONS];
	int num_records;
};

person_directory* person_directory_create(void)
{
	person_directory* pd = xmalloc(sizeof(person_directory));
	return pd;
}

void person_directory_cleanup(person_directory* pd)
{
	for(int i = 0; i < pd->num_records; i++) {
		xfree(pd->records[i].person);
	}

	xfree(pd);
}

person* person_directory_get_person_at(person_directory* pd, int mx, int my, int dx, int dy)
{
	for(int i = 0; i < pd->num_records; i++) {
		person_record* r = &pd->records[i];
		if(r->mx == mx && r->my == my && r->dx == dx && r->dy == dy)
			return r->person;
	}

	return NULL;
}

int person_directory_get_num_people(const person_directory* pd, int mx, int my)
{
	int ret = 0;
	for(int i = 0; i < pd->num_records; i++) {
		const person_record* r = &pd->records[i];
		if(r->mx == mx && r->my == my)
			ret++;
	}

	return ret;
}

int person_directory_get_people(const person_directory* pd, int mx, int my, const person** people, unsigned int bufsiz)
{
	int numret = 0;

	if(bufsiz == 0)
		return 0;

	for(int i = 0; i < pd->num_records; i++) {
		const person_record* r = &pd->records[i];
		if(r->mx == mx && r->my == my) {
			people[numret++] = r->person;
			if(numret == bufsiz)
				return numret;
		}
	}

	return numret;
}

void person_directory_get_person_position(const person_directory* pd, const person* p, int* dx, int* dy)
{
	for(int i = 0; i < pd->num_records; i++) {
		const person_record* r = &pd->records[i];
		if(p == r->person) {
			*dx = r->dx;
			*dy = r->dy;
			return;
		}
	}
	assert(0);
}

void person_directory_add_person(person_directory* pd, int mx, int my, int dx, int dy, person* p)
{
	assert(pd->num_records < MAX_NUM_PERSONS);
	person_record* r = &pd->records[pd->num_records++];
	r->person = p;
	r->mx = mx;
	r->my = my;
	r->dx = dx;
	r->dy = dy;
}

void person_directory_act(person_directory* pd, int mx, int my, const detmap* detm, int px, int py)
{
	for(int i = 0; i < pd->num_records; i++) {
		person_record* r = &pd->records[i];
		if(r->mx == mx && r->my == my) {
			assert(r->person);
			int want_move = my_rand() % 4 == 0;
			if(want_move) {
				int nx = my_rand() % 3 - 1;
				int ny = my_rand() % 3 - 1;
				if(nx || ny) {
					int new_x = r->dx + nx;
					int new_y = r->dy + ny;
					if(new_x != px && new_y != py) {
						if(detmap_passable(detm, new_x, new_y) &&
								!person_directory_get_person_at(pd, mx, my, new_x, new_y)) {
							r->dx = new_x;
							r->dy = new_y;
						}
					}
				}
			}
		}
	}
}

void person_directory_remove_persons_at(person_directory* pd, int mx, int my)
{
	for(int i = 0; i < pd->num_records; i++) {
		person_record* r = &pd->records[i];
		if(r->mx == mx && r->my == my) {
			xfree(r->person);

			if(i != pd->num_records - 1) {
				pd->records[i] = pd->records[pd->num_records - 1];
			}
			pd->num_records--;
			i--;
		}
	}
}


