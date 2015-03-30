#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdlib.h>

typedef struct {
	size_t cap;
	size_t len;
	void **buf;
} vector;

typedef struct {
	void *key;
	void *val;
} pair;

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

#endif
