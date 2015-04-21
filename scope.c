#include <stdlib.h>
#include <assert.h>

#include "scope.h"

scope *scope_new_root(void) {
	scope *res = malloc(sizeof(scope));
	res->parent = NULL;
	vec_init(&res->children);
	vec_init(&res->syms);
	return res;
}

scope *scope_new(scope *parent) {
	scope *res = scope_new_root();
	res->parent = parent;
	vec_insert(&parent->children, 0, res);
	return res;
}


