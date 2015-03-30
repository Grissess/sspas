#include <stdlib.h>
#include <assert.h>

#include "type.h"
#include "vector.h"
#include "util.h"

type *type_new(void) {
	type *res = malloc(sizeof(struct _type));
	assert(res);
	res->refcnt = 1;
	return res;
}

type *type_new_int(void) {
	type *res = type_new();
	res->kind = TP_INT;
	return res;
}

type *type_new_real(void) {
	type *res = type_new();
	res->kind = TP_REAL;
	return res;
}

type *type_new_array(type *base, ssize_t lbound, ssize_t ubound) {
	type *res = type_new();
	res->kind = TP_ARRAY;
	res->base = base;
	res->lbound = lbound;
	res->ubound = ubound;
	return res;
}

type *type_new_func(type *ret, vector *args) {
	type *res = type_new();
	res->kind = TP_FUNC;
	res->ret = ret;
    vec_init(&res->args);
	vec_map(args, &res->args, type_copy);
	return res;
}

type *type_new_struct(vector *names, vector *types) {
	type *res = type_new();
	res->kind = TP_STRUCT;
	vec_init(&res->names);
	vec_init(&res->types);
	vec_map(names, &res->names, strdup);
	vec_map(types, &res->types, type_copy);
	return res;
}

type *type_new_union(vector *names, vector *types) {
	type *res = type_new_struct(names, types);
	res->kind = TP_UNION;
	return res;
}

type *type_copy(type *tp) {
	tp->refcnt++;
	return tp;
}

void type_delete(type *tp) {
	if(!(tp->refcnt--)) {
		type_destroy(tp);
	}
}

void type_destroy(type *tp) {
	switch(tp->kind) {
		case TP_INT: case TP_REAL: case TP_BOOL:
			break;

		case TP_ARRAY:
			type_delete(tp->base);
			break;

		case TP_FUNC:
			type_delete(tp->ret);
			vec_foreach(&tp->args, type_delete);
			break;

		case TP_STRUCT: case TP_UNION:
			vec_foreach(&tp->names, free);
			vec_foreach(&tp->types, type_delete);
			break;

		default:
			assert(0);
	}
	free(tp);
}


int type_equal(type *tpa, type *tpb) {
	if(tpa->kind != tpb->kind) {
		return 0;
	}
	switch(tpa->kind) {
		case TP_INT: case TP_REAL: case TP_BOOL:
			break;

		case TP_ARRAY:
			if(!type_equal(tpa->base, tpb->base)) {
				return 0;
			}
			break;

		case TP_FUNC:
			if(!type_equal(tpa->ret, tpb->ret)) {
				return 0;
			}
			if(!vec_equal(&tpa->args, &tpb->args, type_equal)) {
				return 0;
			}
			break;

		case TP_STRUCT: case TP_UNION:
			if(!vec_equal(&tpa->names, &tpb->names, string_equal)) {
				return 0;
			}
			if(!vec_equal(&tpa->types, &tpb->types, type_equal)) {
				return 0;
			}
			break;
	}
	return 1;
}
