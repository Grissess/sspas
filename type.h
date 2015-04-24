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
	};
} type;

type *type_new(void);
type *type_new_int(void);
type *type_new_real(void);
type *type_new_char(void);
type *type_new_array(type *base,ssize_t lbound,ssize_t size);
type *type_new_func(type *ret,vector *args);
type *type_new_struct(vector *names,vector *types);
type *type_new_union(vector *names,vector *types);
type *type_copy(type *tp);
void type_delete(type *tp);
void type_destroy(type *tp);
int type_equal(type *tpa,type *tpb);

#endif
