#include <assert.h>
#include <stdlib.h>

#include "ast.h"

expr_node *ex_new(void) {
	expr_node *res = malloc(sizeof(expr_node));
	assert(res);
	res->refcnt = 1;
	return res;
}

expr_node *ex_copy(expr_node *ex) {
	ex->refcnt++;
	return ex;
}

expr_node *ex_new_assign(char *name, expr_node *value) {
	expr_node *res = ex_new();
	res->kind = EX_ASSIGN;
	res->assign.ident = strdup(name);

