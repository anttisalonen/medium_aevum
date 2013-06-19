#ifndef MA_PERSON_H
#define MA_PERSON_H

struct person;
typedef struct person person;

person* person_create(void);
void person_cleanup(person* p);

#endif

