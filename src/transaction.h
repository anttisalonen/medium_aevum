#ifndef MA_TRANSACTION_H
#define MA_TRANSACTION_H

typedef enum {
	transaction_none,
	transaction_give_food,
} transaction_type;

typedef struct {
	transaction_type type;
	union {
		struct {
			int howmuch;
		} give_food;
	} data;
} transaction;

#endif

