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
	if(v->cap >= v->len) {
		v->cap = v->cap * 2 + 1;
		v->buf = realloc(v->buf, v->cap);
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
	v->buf = realloc(v->buf, cap);
	if(cap) assert(v->buf);
	v->cap = cap;
	v->len = (v->cap < v->len ? v->cap : v->len);
}

void vec_map(vector *vin, vector *vout, void *(*map)(void *)) {
	size_t i;
	vec_clear(vout);
	vec_alloc(vout, vin->len);
	for(i = 0; i < vin->len; i++) {
		vout->buf[i] = map(vin->buf[i]);
	}
}

void *vec_reduce(vector *v, void *(*reduce)(void *, void *), void *init) {
	void *accum = init;
	size_t i;
	for(i = 0; i < v->len; i++) {
		accum = reduce(accum, v->buf[i]);
	}
	return accum;
}

int vec_test(vector *v, int (*test)(void *, void *, vector *, size_t), void *data) {
	size_t i;
	int res;
	for(i = 0; i < v->len; i++) {
		if((res = test(data, v->buf[i], v, i))) return res;
	}
	return 0;
}

int vec_equal(vector *va, vector *vb, int (*eq)(void *, void *)) {
	size_t i;
	if(va->len != vb->len) return 0;
	for(i = 0; i < va->len; i++) {
		if(!eq(va->buf[i], vb->buf[i])) {
			return 0;
		}
	}
	return 1;
}

void vec_foreach(vector *v, void (*iter)(void *)) {
	size_t i;
	for(i = 0; i < v->len; i++) {
		iter(v->buf[i]);
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
}

