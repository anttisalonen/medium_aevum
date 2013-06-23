#ifndef MA_PERSON_DIRECTORY_H
#define MA_PERSON_DIRECTORY_H

#include "person.h"
#include "detailed_map.h"

struct person_directory;
typedef struct person_directory person_directory;

/* init */
person_directory* person_directory_create(void);
void person_directory_cleanup(person_directory* pd);

/* lookup */
person* person_directory_get_person_at(person_directory* pd, int mx, int my, int dx, int dy);
int person_directory_get_num_people(const person_directory* pd, int mx, int my);
int person_directory_get_people(const person_directory* pd, int mx, int my, const person** people, unsigned int bufsiz);

/* information */
void person_directory_get_person_position(const person_directory* pd, const person* p, int* dx, int* dy);

/* addition */
void person_directory_add_person(person_directory* pd, int mx, int my, int dx, int dy, person* p);

/* action */
void person_directory_act(person_directory* pd, int mx, int my, const detmap* detm);

#endif

