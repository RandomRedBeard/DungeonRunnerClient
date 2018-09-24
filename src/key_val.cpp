#include "key_val.h"

key_val::key_val(char* k, int* v) {
	key = k;
	val = v;
}

int key_val::sprint(char* buffer, int len) {
	return snprintf(buffer, len, "%s - \"%c\"\n", key, *val);
}
