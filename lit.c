#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "lit.h"
#include "vector.h"
#include "type.h"
#include "util.h"

literal *lit_new(void) {
	literal *lit = malloc(sizeof(literal));
	lit->refcnt = 1;
	return lit;
}

literal *lit_copy(literal *lit) {
	lit->refcnt++;
	return lit;
}

literal *lit_new_int(long ival) {
	literal *lit = lit_new();
	lit->kind = LIT_INT;
	lit->type = type_new_int();
	lit->ival = ival;
	return lit;
}

literal *lit_new_real(double fval) {
	literal *lit = lit_new();
	lit->kind = LIT_REAL;
	lit->type = type_new_real();
	lit->fval = fval;
	return lit;
}

literal *lit_new_char(char cval) {
	literal *lit = lit_new();
	lit->kind = LIT_CHAR;
	lit->type = type_new_char();
	lit->cval = cval;
	return lit;
}

literal *lit_new_array(vector *init, type *fallback) {
	literal *lit = lit_new();
	lit->kind = LIT_ARRAY;
	vec_init(&lit->items);
	if(init) vec_map(init, &lit->items, (vec_map_f) lit_copy, NULL);
	if(!fallback && lit->items.len > 0) fallback = vec_get(&lit->items, 0, literal)->type;
	if(!fallback) fallback = type_new_int(); /* FIXME */
	lit->type = type_new_array(fallback, 0, lit->items.len);
	return lit;
}

literal *lit_new_range(long lbound, size_t size) {
	literal *lit = lit_new();
	long i;
	lit->kind = LIT_ARRAY;
	vec_init(&lit->items);
	vec_alloc(&lit->items, size);
	for(i = 0; i < size; i++) {
		vec_insert(&lit->items, i, lit_new_int(i));
	}
	lit->type = type_new_array(type_new_int(), 0, size);
	return lit;
}

void lit_array_append(literal *arr, literal *lit) {
	vec_insert(&arr->items, arr->items.len, lit_copy(lit));
}

void lit_delete(literal *lit) {
	if(!(--lit->refcnt)) {
		lit_destroy(lit);
	}
}
void lit_destroy(literal *lit) {
	switch(lit->kind) {
		case LIT_INT: case LIT_REAL: case LIT_CHAR:
			break;

		case LIT_ARRAY:
			vec_foreach(&lit->items, (vec_iter_f) lit_delete, NULL);
			vec_clear(&lit->items);
			break;

		default:
			assert(0);
	}
	type_delete(lit->type);
	free(lit);
}

void lit_print(FILE *out, int lev, literal *lit) {
	size_t i;
	if(!lit) {
		wrlev(out, lev, "{NULL}");
		return;
	}
	switch(lit->kind) {
		case LIT_INT:
			wrlev(out, lev, "{Integer (%s): %ld}", type_repr(lit->type), lit->ival);
			break;

		case LIT_REAL:
			wrlev(out, lev, "{Real (%s): %f}", type_repr(lit->type), lit->fval);
			break;

		case LIT_CHAR:
			wrlev(out, lev, "{Character (%s): %c}", type_repr(lit->type), lit->cval);
			break;

		case LIT_ARRAY:
			wrlev(out, lev, "{Array: %s}", type_repr(lit->type));
			for(i = 0; i < lit->items.len; i++) {
				lit_print(out, lev+1, vec_get(&lit->items, i, literal));
			}
			break;

		default:
			wrlev(out, lev, "!!!{UNKNOWN LITERAL}!!!");
			break;
	}
}
