#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "vector.h"

void vec_init(vector *v) {
	v->cap = 0;
	v->len = 0;
	v->buf = NULL;
}

void vec_clear(vector *v) {
	free(v->buf);
	vec_init(v);
}

void vec_insert(vector *v, size_t idx, void *val) {
	if(idx > v->len) return;
	if(v->len >= v->cap) {
		v->cap = v->cap * 2 + 1;
		v->buf = realloc(v->buf, v->cap * sizeof(void *));
		if(v->cap) assert(v->buf);
	}
	memmove(v->buf + (idx + 1), v->buf + idx, (v->len - idx) * sizeof(void *));
	v->buf[idx] = val;
	v->len++;
}

void *vec_remove(vector *v, size_t idx) {
	void *temp;
	if(idx >= v->len) return NULL;
	temp = v->buf[idx];
	v->len--;
	memmove(v->buf + idx, v->buf + (idx + 1), (v->len - idx) * sizeof(void *));
	return temp;
}

void vec_alloc(vector *v, size_t cap) {
	v->buf = realloc(v->buf, cap * sizeof(void *));
	if(cap) assert(v->buf);
	v->cap = cap;
	v->len = (v->cap < v->len ? v->cap : v->len);
}

void vec_map(vector *vin, vector *vout, vec_map_f map, void *data) {
	size_t i;
	vec_clear(vout);
	vec_alloc(vout, vin->len);
	for(i = 0; i < vin->len; i++) {
		vout->buf[i] = map(vin->buf[i], data);
	}
        vout->len = vin->len; /* XXX Race condition if map modifies vout */
}

void *vec_reduce(vector *v, vec_reduce_f reduce, void *init) {
	void *accum = init;
	size_t i;
	for(i = 0; i < v->len; i++) {
		accum = reduce(accum, v->buf[i]);
	}
	return accum;
}

int vec_test(vector *v, vec_test_f test, void *data) {
	size_t i;
	int res;
	for(i = 0; i < v->len; i++) {
		if((res = test(data, v->buf[i], v, i))) return res;
	}
	return 0;
}

int vec_equal(vector *va, vector *vb, vec_eq_f eq) {
	size_t i;
	if(va->len != vb->len) return 0;
	for(i = 0; i < va->len; i++) {
		if(!eq(va->buf[i], vb->buf[i])) {
			return 0;
		}
	}
	return 1;
}

void vec_foreach(vector *v, vec_iter_f iter, void *data) {
	size_t i;
	for(i = 0; i < v->len; i++) {
		iter(v->buf[i], data);
	}
}

ssize_t vec_search(vector *v, void *val) {
	size_t i;
	for(i = 0; i < v->len; i++) {
		if(val == v->buf[i]) return i;
	}
	return -1;
}

void vec_copy(vector *from, vector *into) {
	size_t i;
	vec_clear(into);
	vec_alloc(into, from->len);
	for(i = 0; i < from->len; i++) {
		into->buf[i] = from->buf[i];
	}
        into->len = from->len; /* But also see vec_map */
}

#if !defined(VECTOR_NO_GETSET) && (defined(VECTOR_GETSET_FUNCS) || defined(VECTOR_GETSET_BOUNDS))

/* Hopefully, intelligent optimizers will lower this for the trivial case. These
 * might be useful with function pointers, though.
 */

void *vec_get2(vector *v, size_t idx) {
#ifdef VECTOR_GETSET_BOUNDS
    if(idx >= v->len) return NULL;
#endif
    return v->buf[idx];
}

void *vec_get(vector *v, size_t idx) {
#ifdef VECTOR_GETSET_BOUNDS
    if(idx >= v->len) return NULL;
#endif
    return vec_get2(v, idx);
}

void *vec_set(vector *v, size_t idx, void *val) {
#ifdef VECTOR_GETSET_BOUNDS
    if(idx >= v->len) return val;
#endif
    return (v->buf[idx] = val); /* For compat with macro definition */
}

#endif
