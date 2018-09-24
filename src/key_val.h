#ifndef KEY_VAL_H_
#define KEY_VAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct key_val {
	char* key;
	int* val;
	int sprint(char*, int);
	key_val(char*, int*);
};

#endif
