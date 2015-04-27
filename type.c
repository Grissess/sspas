#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "type.h"
#include "vector.h"
#include "util.h"

type *type_new(void) {
	type *res = malloc(sizeof(struct _type));
	assert(res);
	res->refcnt = 1;
	return res;
}

type *_type_simple_factory(type_k kind) {
	type *res = type_new();
	res->kind = kind;
	return res;
}

type *type_new_int(void) {
	static type *res = NULL;
	if(!res) res = _type_simple_factory(TP_INT);
	return type_copy(res);
}

type *type_new_real(void) {
	static type *res = NULL;
	if(!res) res = _type_simple_factory(TP_REAL);
	return type_copy(res);
}

type *type_new_char(void) {
	static type *res = NULL;
	if(!res) res = _type_simple_factory(TP_CHAR);
	return type_copy(res);
}

type *type_new_array(type *base, ssize_t lbound, ssize_t size) {
	type *res = type_new();
	res->kind = TP_ARRAY;
	res->base = type_copy(base);
	res->lbound = lbound;
	res->size = size;
	return res;
}

type *type_new_func(type *ret, vector *args) {
	type *res = type_new();
	res->kind = TP_FUNC;
	if(ret) res->ret = type_copy(ret);
	else res->ret = NULL;
    vec_init(&res->args);
	vec_map(args, &res->args, (vec_map_f) type_copy, NULL);
	return res;
}

type *type_new_struct(vector *names, vector *types) {
	type *res = type_new();
	res->kind = TP_STRUCT;
	vec_init(&res->names);
	vec_init(&res->types);
	vec_map(names, &res->names, (vec_map_f) strdup, NULL);
	vec_map(types, &res->types, (vec_map_f) type_copy, NULL);
	return res;
}

type *type_new_union(vector *names, vector *types) {
	type *res = type_new_struct(names, types);
	res->kind = TP_UNION;
	return res;
}

type *type_new_ref(const char *ref) {
	type *res = type_new();
	res->kind = TP_REF;
	res->ref = strdup(ref);
	return res;
}

type *type_copy(type *tp) {
	tp->refcnt++;
	return tp;
}

void type_delete(type *tp) {
	if(!(--tp->refcnt)) {
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
			vec_foreach(&tp->args, (vec_iter_f) type_delete, NULL);
			vec_clear(&tp->args);
			break;

		case TP_STRUCT: case TP_UNION:
			vec_foreach(&tp->names, (vec_iter_f) free, NULL);
			vec_clear(&tp->names);
			vec_foreach(&tp->types, (vec_iter_f) type_delete, NULL);
			vec_clear(&tp->types);
			break;

		case TP_REF:
			free(tp->ref);
			break;

		default:
			assert(0);
			break;
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
			if(!vec_equal(&tpa->args, &tpb->args, (vec_eq_f) type_equal)) {
				return 0;
			}
			break;

		case TP_STRUCT: case TP_UNION:
			if(!vec_equal(&tpa->names, &tpb->names, (vec_eq_f) string_equal)) {
				return 0;
			}
			if(!vec_equal(&tpa->types, &tpb->types, (vec_eq_f) type_equal)) {
				return 0;
			}
			break;
	}
	return 1;
}

#define TREPR_SZ 1024
static char trepr[TREPR_SZ];

char *type_repr(type *ty) {
	char tbuffer[TREPR_SZ]={0};
	int chars;
	size_t i;
	if(!ty) {
		snprintf(trepr, TREPR_SZ, "NULL");
		return trepr;
	}
	switch(ty->kind) {
		case TP_INT:
			chars = snprintf(tbuffer, TREPR_SZ, "integer");
			break;

		case TP_REAL:
			chars = snprintf(tbuffer, TREPR_SZ, "real");
			break;

		case TP_CHAR:
			chars = snprintf(tbuffer, TREPR_SZ, "character");
			break;

		case TP_ARRAY:
			chars = snprintf(tbuffer, TREPR_SZ, "array[%ld..%ld] of %s", ty->lbound, ty->lbound + ty->size, type_repr(ty->base));
			break;

		case TP_BOOL:
			chars = snprintf(tbuffer, TREPR_SZ, "(bool)");
			break;

		case TP_FUNC:
			chars = snprintf(tbuffer, TREPR_SZ, "(");
			for(i = 0; i < ty->args.len; i++) {
				chars += snprintf(tbuffer+chars, TREPR_SZ-chars, "%s,", type_repr(vec_get(&ty->args, i, type)));
			}
			chars += snprintf(tbuffer+chars, TREPR_SZ-chars, ")->%s", type_repr(ty->ret));
			break;

		case TP_STRUCT:
			chars = snprintf(tbuffer, TREPR_SZ, "struct (");
			for(i = 0; i < ty->types.len; i++) {
				chars += snprintf(tbuffer+chars, TREPR_SZ-chars, "%s: %s,", vec_get(&ty->names, i, char), type_repr(vec_get(&ty->types, i, type)));
			}
			chars += snprintf(tbuffer+chars, TREPR_SZ-chars, ")");
			break;

		case TP_UNION:
			chars = snprintf(tbuffer, TREPR_SZ, "union (");
			for(i = 0; i < ty->types.len; i++) {
				chars += snprintf(tbuffer+chars, TREPR_SZ-chars, "%s: %s,", vec_get(&ty->names, i, char), type_repr(vec_get(&ty->types, i, type)));
			}
			chars += snprintf(tbuffer+chars, TREPR_SZ-chars, ")");
			break;

		case TP_REF:
			chars = snprintf(tbuffer, TREPR_SZ, "(ref: %s)", ty->ref);
			break;

		default:
			chars = snprintf(tbuffer, TREPR_SZ, "!!!UNKNOWN TYPE!!!");
			break;
	}
	memmove(trepr, tbuffer, chars+1);
	return trepr;
}
