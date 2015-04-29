#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>
#include <stdlib.h>

#include "vector.h"

typedef enum {
	TP_INT,
	TP_REAL,
	TP_CHAR,
	TP_ARRAY,
	TP_BOOL,
	TP_FUNC,
	TP_STRUCT,
	TP_UNION,
	TP_REF,
} type_k;

typedef struct _type type;

typedef struct _type {
	type_k kind;
	size_t refcnt;
	union {
		struct {
			type *base;
			ssize_t lbound;
			ssize_t size;
		};
		struct {
			type *ret;
			vector args; /* of type * */
		};
		struct {
			vector names; /* of char * */
			vector types; /* of type * */
		};
		char *ref;
	};
} type;

type *type_new(void);
type *type_new_int(void);
type *type_new_real(void);
type *type_new_char(void);
type *type_new_bool(void);
type *type_new_array(type *base, ssize_t lbound, ssize_t size);
type *type_new_func(type *ret, vector *args);
type *type_new_struct(vector *names, vector *types);
type *type_new_union(vector *names, vector *types);
type *type_new_ref(const char *ref);
type *type_copy(type *tp);
void type_delete(type *tp);
void type_destroy(type *tp);
int type_equal(type *tpa, type *tpb);
const char *type_repr(type *ty);

typedef enum {
	CAST_NONE,
	CAST_EXPLICIT,
	CAST_UNINTENDED,
	CAST_IMPLICIT,
} cast_k;

cast_k type_can_cast(type *from,type *to);
type *type_num_promote(type *ta,type *tb);
cast_k type_can_index(type *object,type *index);
type *type_of_index(type *object,type *index);
cast_k type_can_setindex(type *object,type *index,type *value);
cast_k type_can_call(type *func,vector *params);
type *type_of_call(type *func,vector *params);
cast_k type_can_unop(type *value,int kind);
type *type_of_unop(type *value,int kind);
cast_k type_can_binop(type *left,int kind,type *right);
type *type_of_binop(type *left,int kind,type *right);
cast_k type_can_iter(type *ty);

#endif
