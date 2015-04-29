#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "type.h"
#include "vector.h"
#include "util.h"
#include "ast.h"

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
	if(!res) {
		res = _type_simple_factory(TP_INT);
	}
	return type_copy(res);
}

type *type_new_real(void) {
	static type *res = NULL;
	if(!res) {
		res = _type_simple_factory(TP_REAL);
	}
	return type_copy(res);
}

type *type_new_char(void) {
	static type *res = NULL;
	if(!res) {
		res = _type_simple_factory(TP_CHAR);
	}
	return type_copy(res);
}

type *type_new_bool(void) {
	static type *res = NULL;
	if(!res) {
		res = _type_simple_factory(TP_BOOL);
	}
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
	if(ret) {
		res->ret = type_copy(ret);
	} else {
		res->ret = NULL;
	}
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
		case TP_INT:
		case TP_REAL:
		case TP_BOOL:
			break;

		case TP_ARRAY:
			type_delete(tp->base);
			break;

		case TP_FUNC:
			type_delete(tp->ret);
			vec_foreach(&tp->args, (vec_iter_f) type_delete, NULL);
			vec_clear(&tp->args);
			break;

		case TP_STRUCT:
		case TP_UNION:
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
	if(!tpa || !tpb) {
		if(!tpa && !tpb) return 1;
		return 0;
	}
	if(tpa->kind != tpb->kind) {
		return 0;
	}
	switch(tpa->kind) {
		case TP_INT:
		case TP_REAL:
		case TP_BOOL:
		case TP_CHAR:
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

		case TP_STRUCT:
		case TP_UNION:
			if(!vec_equal(&tpa->names, &tpb->names, (vec_eq_f) string_equal)) {
				return 0;
			}
			if(!vec_equal(&tpa->types, &tpb->types, (vec_eq_f) type_equal)) {
				return 0;
			}
			break;

		case TP_REF:
			return string_equal(tpa->ref, tpb->ref);
			break;
	}
	return 1;
}

#define TREPR_SZ 1024

const char *type_repr(type *ty) {
	char tbuffer[TREPR_SZ] = {0};
	char *trepr = malloc(sizeof(char) * TREPR_SZ);
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
				chars += snprintf(tbuffer + chars, TREPR_SZ - chars, "%s,", type_repr(vec_get(&ty->args, i, type)));
			}
			chars += snprintf(tbuffer + chars, TREPR_SZ - chars, ")->%s", type_repr(ty->ret));
			break;

		case TP_STRUCT:
			chars = snprintf(tbuffer, TREPR_SZ, "struct (");
			for(i = 0; i < ty->types.len; i++) {
				chars += snprintf(tbuffer + chars, TREPR_SZ - chars, "%s: %s,", vec_get(&ty->names, i, char), type_repr(vec_get(&ty->types, i, type)));
			}
			chars += snprintf(tbuffer + chars, TREPR_SZ - chars, ")");
			break;

		case TP_UNION:
			chars = snprintf(tbuffer, TREPR_SZ, "union (");
			for(i = 0; i < ty->types.len; i++) {
				chars += snprintf(tbuffer + chars, TREPR_SZ - chars, "%s: %s,", vec_get(&ty->names, i, char), type_repr(vec_get(&ty->types, i, type)));
			}
			chars += snprintf(tbuffer + chars, TREPR_SZ - chars, ")");
			break;

		case TP_REF:
			chars = snprintf(tbuffer, TREPR_SZ, "(ref: %s)", ty->ref);
			break;

		default:
			chars = snprintf(tbuffer, TREPR_SZ, "!!!UNKNOWN TYPE!!!");
			break;
	}
	memcpy(trepr, tbuffer, chars + 1);
	return trepr;
}

int num_rank[] = {
	2,					/*TP_INT*/
	3,					/*TP_REAL*/
	1,					/*TP_CHAR*/
	-1,					/*TP_ARRAY*/
	0,					/*TP_BOOL*/
	-1,					/*TP_FUNC*/
	-1,					/*TP_STRUCT*/
	-1,					/*TP_UNION*/
	-1,					/*TP_REF*/
};

cast_k type_can_cast(type *from, type *to) {
	if(!from) return CAST_NONE;
	switch(to->kind) {
		case TP_INT:
		case TP_REAL:
		case TP_CHAR:
		case TP_BOOL:
			if(from->kind == TP_INT || from->kind == TP_REAL || from->kind == TP_CHAR || from->kind == TP_BOOL) {
				int flev = num_rank[from->kind], tlev = num_rank[to->kind];
				assert(flev >= 0 && tlev >= 0);
				if(tlev >= flev) {
					return CAST_IMPLICIT;
				}
				return CAST_UNINTENDED;
			}
			return CAST_NONE;
			break;

		case TP_ARRAY:
			if(to->size >= 0) {
				if(!type_equal(from->base, to->base)) {
					return CAST_UNINTENDED;
				}
				if(from->size < to->size) {
					return CAST_UNINTENDED;
				}
				return CAST_IMPLICIT;
			}
			if(!type_equal(from->base, to->base)) {
				return CAST_UNINTENDED;
			}
			return CAST_IMPLICIT;
			break;

		case TP_FUNC:
		case TP_STRUCT:
		case TP_UNION:
			if(type_equal(from, to)) {
				return CAST_IMPLICIT;
			}
			return CAST_EXPLICIT;
			break;

		default: /* case TYPE_REF */
			assert(0);
			break;
	}
}

type *type_num_promote(type *ta, type *tb) {
	int alev = num_rank[ta->kind], blev = num_rank[tb->kind];
	assert(alev >= 0 && blev >= 0);
	if(alev > blev) {
		return ta;
	}
	return tb;
}

cast_k type_can_index(type *object, type *index) {
	switch(object->kind) {
		case TP_ARRAY:
			return type_can_cast(index, type_new_int());
			break;

		default:
			return CAST_NONE;
			break;
	}
}

type *type_of_index(type *object, type *index) {
	switch(object->kind) {
		case TP_ARRAY:
			return object->base;
			break;

		default:
			assert(0);
			break;
	}
}

cast_k type_can_setindex(type *object, type *index, type *value) {
	switch(object->kind) {
		case TP_ARRAY:
			if(type_can_cast(index, type_new_int()) >= CAST_UNINTENDED) {
				return type_can_cast(value, object->base);
			}
			return CAST_NONE;
			break;

		default:
			return CAST_NONE;
			break;
	}
}

/* params of type * */
cast_k type_can_call(type *func, vector *params) {
	size_t i;
	cast_k c = CAST_UNINTENDED;
	switch(func->kind) {
		case TP_FUNC:
			if(type_equal(func, type_new_func(func->ret, params))) {
				return CAST_IMPLICIT;
			}
			if(params->len != func->args.len) {
				return CAST_NONE;
			}
			for(i = 0; i < params->len; i++) {
				c = min(c, type_can_cast(vec_get(params, i, type), vec_get(&func->args, i, type)));
			}
			return c;
			break;

		default:
			return CAST_NONE;
			break;
	}
}

type *type_of_call(type *func, vector *params) {
	switch(func->kind) {
		case TP_FUNC:
			return func->ret;
			break;

		default:
			assert(0);
			break;
	}
}

cast_k type_can_unop(type *value, int kind) {
	switch(kind) {
		case OP_NEG:
			return type_can_cast(value, type_new_real());
			break;

		case OP_NOT:
			return type_can_cast(value, type_new_bool());
			break;

		case OP_BNOT:
			return type_can_cast(value, type_new_int());
			break;

		case OP_IDENT:
			return CAST_IMPLICIT;
			break;

		default:
			assert(0);
			break;
	}
}

type *type_of_unop(type *value, int kind) {
	switch(kind) {
		case OP_NEG:
		case OP_IDENT:
			return value;
			break;

		case OP_NOT:
			return type_new_bool();
			break;

		case OP_BNOT:
			return type_new_int();
			break;

		default:
			assert(0);
			break;
	}
}

cast_k type_can_binop(type *left, int kind, type *right) {
	switch(kind) {
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_MOD:
			return min(type_can_cast(left, type_new_real()), type_can_cast(left, right));
			break;

		case OP_EQ:
		case OP_NEQ:
			return CAST_IMPLICIT;
			break;

		case OP_LESS:
		case OP_GREATER:
		case OP_LEQ:
		case OP_GEQ:
			return min(type_can_cast(left, type_new_real()), type_can_cast(left, right));
			break;

		case OP_AND:
		case OP_OR:
			return min(type_can_cast(left, type_new_bool()), type_can_cast(left, right));
			break;

		case OP_BAND:
		case OP_BOR:
		case OP_BXOR:
		case OP_BLSHIFT:
		case OP_BRSHIFT:
			return min(type_can_cast(left, type_new_int()), type_can_cast(left, right));
			break;

		default:
			assert(0);
			break;
	}
}

type *type_of_binop(type *left, int kind, type *right) {
	switch(kind) {
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_MOD:
			return type_num_promote(left, right);
			break;

		case OP_EQ:
		case OP_NEQ:
		case OP_LESS:
		case OP_GREATER:
		case OP_LEQ:
		case OP_GEQ:
		case OP_AND:
		case OP_OR:
			return type_new_bool();
			break;

		case OP_BAND:
		case OP_BOR:
		case OP_BXOR:
		case OP_BLSHIFT:
		case OP_BRSHIFT:
			return type_new_int();
			break;

		default:
			assert(0);
			break;
	}
}

cast_k type_can_iter(type *ty) {
	switch(ty->kind) {
		case TP_ARRAY:
			if(ty->size >= 0) {
				return CAST_IMPLICIT;
			}
			return CAST_NONE;
			break;

		default:
			return CAST_NONE;
			break;
	}
}
