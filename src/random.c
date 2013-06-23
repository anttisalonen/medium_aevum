#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "random.h"

#define RAND_STACK_SIZE 16

static int rands[RAND_STACK_SIZE];
static int pos_rand;

static void my_srand(int x)
{
	srand(x);
}

void my_rand_push(int x)
{
	if(pos_rand >= RAND_STACK_SIZE) {
		fprintf(stderr, "Too many items in random stack!\n");
		abort();
	}
	my_srand(x);
	pos_rand++;
	rand();
}

void my_rand_pop()
{
	if(pos_rand == 0) {
		fprintf(stderr, "No items in random stack!\n");
		abort();
	}
	pos_rand--;
	my_srand(rands[pos_rand]);
}

int my_rand()
{
	int r = rand();
	rands[pos_rand] = r;
	return r;
}


