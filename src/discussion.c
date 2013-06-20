#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "xmalloc.h"
#include "discussion.h"

struct discussion {
	char* line;
	char** answers;
	int num_answers;
	int have_food;
};

char* alloc_line(const char* s)
{
	assert(strlen(s) < 128);
	char* ptr = xmalloc(128);
	snprintf(ptr, 127, "%s", s);
	ptr[127] = 0;
	return ptr;
}

void set_line(discussion* d, const char* s)
{
	free(d->line);
	d->line = alloc_line(s);
}

discussion* discussion_create(int can_give_food)
{
	discussion* d = xmalloc(sizeof(discussion));
	d->have_food = can_give_food;
	if(can_give_food) {
		set_line(d, "Would you like to have some food?");

		d->num_answers = 3;
		d->answers = xmalloc(d->num_answers * sizeof(char*));
		d->answers[0] = alloc_line("Where am I?");
		d->answers[1] = alloc_line("Yes please.");
		d->answers[2] = alloc_line("No thanks.");
	} else {
		set_line(d, "I unfortunately have no more food to give.");

		d->num_answers = 2;
		d->answers = xmalloc(d->num_answers * sizeof(char*));
		d->answers[0] = alloc_line("Where am I?");
		d->answers[1] = alloc_line("Ok then.");
	}

	return d;
}

static void free_answers(discussion* d)
{
	for(int i = 0; i < d->num_answers; i++)
		xfree(d->answers[i]);
	xfree(d->answers);
	d->answers = NULL;
	d->num_answers = 0;
}

void discussion_cleanup(discussion* d)
{
	free_answers(d);
	xfree(d->line);
	xfree(d);
}

const char* discussion_get_line(discussion* d)
{
	return d->line;
}

int discussion_get_answers(const discussion* d, char*** answers)
{
	if(!d->num_answers)
		return 0;

	*answers = d->answers;
	return d->num_answers;
}

transaction empty_transaction(void)
{
	transaction t;
	t.type = transaction_none;
	return t;
}

transaction discussion_give_answer(discussion* d, int n)
{
	free_answers(d);
	if(d->have_food) {
		switch(n) {
			case 0:
				set_line(d, "You're in medieval Europe, of course! What do you think?");
				return empty_transaction();

			case 1:
				{
					transaction t;
					t.type = transaction_give_food;
					t.data.give_food.howmuch = 3;
					set_line(d, "Here you go!");
					return t;
				}

			case 2:
				set_line(d, "Very well then.");
				return empty_transaction();
		}
	} else {
		switch(n) {
			case 0:
				set_line(d, "You're in medieval Europe, of course! What do you think?");
				return empty_transaction();

			case 1:
				set_line(d, "See you around, stranger!");
				return empty_transaction();
		}
	}
	return empty_transaction();
}


