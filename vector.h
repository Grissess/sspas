#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdlib.h>

typedef struct {
	size_t cap;
	size_t len;
	void **buf;
} vector;

void vec_init(vector *v);
void vec_clear(vector *v);
void vec_insert(vector *v,size_t idx,void *val);
void *vec_remove(vector *v,size_t idx);
void vec_alloc(vector *v,size_t cap);
void vec_map(vector *vin,vector *vout,void *(*map)(void *));
void *vec_reduce(vector *v,void *(*reduce)(void *,void *),void *init);
int vec_test(vector *v,int(*test)(void *,void *,vector *,size_t),void *data);
int vec_equal(vector *va,vector *vb,int(*eq)(void *,void *));
void vec_foreach(vector *v,void(*iter)(void *));
ssize_t vec_search(vector *v,void *val);
void vec_copy(vector *from,vector *into);

/* For data encapsulation purists (and perhaps for foresight) */

#ifndef VECTOR_NO_GETSET
#if !defined(VECTOR_GETSET_FUNCS) && !defined(VECTOR_GETSET_BOUNDS)

#include "util/_macro_vararg.h"

#define vec_get2(vec, idx) ((vec).buf[(idx)])
#define vec_get3(vec, idx, tp) (((tp **) (vec).buf)[(idx)])
#define vec_get(...) MVA_DISPATCH(vec_get, __VA_ARGS__)

#define vec_set(vec, idx, val) ((vec).buf[(idx)] = (val))

#else /* !defined(VECTOR_NO_GETSET) && (defined(VECTOR_GETSET_FUNCS)||defined(VECTOR_GETSET_BOUNDS))*/

void *vec_get2(vector *, size_t);
void *vec_get(vector *, size_t);
#define vec_get3(vec, idx, tp) ((tp *) vec_get2((vec), (idx)))
void *vec_set(vector *, size_t, void *);

#endif
#endif

#endif
