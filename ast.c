#include <assert.h>
#include <stdlib.h>

#include "ast.h"
#include "type.h"
#include "vector.h"
#include "lit.h"

expr_node *ex_new(void) {
	expr_node *res = malloc(sizeof(expr_node));
	assert(res);
	res->refcnt = 1;
	res->type = NULL;
	return res;
}

expr_node *ex_copy(expr_node *ex) {
	ex->refcnt++;
	return ex;
}

expr_node *ex_new_lit(literal *lit) {
	expr_node *res = ex_new();
	res->kind = EX_LIT;
	res->lit.lit = lit_copy(lit);
	return res;
}

expr_node *ex_new_assign(char *name, expr_node *value) {
	expr_node *res = ex_new();
	res->kind = EX_ASSIGN;
	res->assign.ident = strdup(name);
	res->assign.value = ex_copy(value);
	return res;
}

expr_node *ex_new_index(expr_node *object, expr_node *index) {
	expr_node *res = ex_new();
	res->kind = EX_INDEX;
	res->index.object = ex_copy(object);
	res->index.index = ex_copy(index);
	return res;
}

expr_node *ex_new_setindex(expr_node *object, expr_node *index, expr_node *value) {
	expr_node *res = ex_new();
	res->kind = EX_SETINDEX;
	res->setindex.object = ex_copy(object);
	res->setindex.index = ex_copy(index);
	res->setindex.value = ex_copy(value);
	return res;
}

expr_node *ex_new_call(expr_node *func, vector *params) {
	expr_node *res = ex_new();
	res->kind = EX_CALL;
	res->call.func = ex_copy(func);
	vec_map(params, &ex->call.params, ex_copy, NULL);
	return res;
}

void ex_delete(expr_node *ex) {
	if(!(ex->refcnt--)) {
		ex_destroy(ex);
	}
}

void ex_destroy(expr_node *ex) {
	if(ex->type) {
		type_delete(ex->type);
	}
	switch(ex->kind) {
		case EX_ASSIGN:
			free(ex->assign.ident);
			ex_delete(ex->assign.value);
			break;

		case EX_INDEX:
			ex_delete(ex->index.object);
			ex_delete(ex->index.index);
			break;

		case EX_SETINDEX:
			ex_delete(ex->setindex.object);
			ex_delete(ex->setindex.index);
			ex_delete(ex->setindex.value);
			break;

		case EX_CALL:
			ex_delete(ex->call.func);
			vec_foreach(&ex->call.params, (vec_iter_f) ex_delete, NULL);
			break;

		default:
			assert(0);
			break;
	}
	free(ex);
}

stmt_node *st_new(void) {
	stmt_node *res = malloc(sizeof(stmt_node));
	res->refcnt = 1;
	return res;
}

stmt_node *st_copy(stmt_node *st) {
	st->refcnt++;
	return st;
}

stmt_node *st_new_expr(expr_node *ex) {
	stmt_node *res = st_new();
	res->kind = ST_EXPR;
	res->expr.expr = ex_copy(ex);
	return res;
}

stmt_node *st_new_while(expr_node *cond, stmt_node *body) {
	stmt_node *res = st_new();
	res->kind = ST_WHILE;
	res->while_.cond = ex_copy(cond);
	res->while_.body = st_copy(body);
	return res;
}

stmt_node *st_new_if(expr_node *cond, stmt_node *iftrue, stmt_node *iffalse) {
	stmt_node *res = st_new();
	res->kind = ST_IF;
	res->if_.cond = ex_copy(cond);
	res->if_.iftrue = st_copy(iftrue);
	res->if_.iffalse = st_copy(iffalse);
	return res;
}

stmt_node *st_new_for(stmt_node *init, expr_node *cond, stmt_node *post, stmt_node *body) {
	stmt_node *res = st_new();
	res->kind = ST_FOR;
	res->for_.init = st_copy(init);
	res->for_.cond = ex_copy(cond);
	res->for_.post = st_copy(post);
	res->for_.body = st_copy(body);
	return res;
}

stmt_node *st_new_iter(expr_node *value, char *ident, stmt_node *body) {
	stmt_node *res = st_new();
	res->kind = ST_ITER;
	res->iter.value = ex_copy(value);
	res->iter.ident = strdup(ident);
	res->iter.body = st_copy(body);
	return res;
}

stmt_node *st_new_compound(vector *stmts) {
	stmt_node *res = st_new();
	res->kind = ST_COMPOUND;
	vec_map(stmts, &res->compound.stmts, (vec_map_f) st_copy, NULL);
	return res;
}

void st_compound_append(stmt_node *cmpd, stmt_node *st) {
	vec_insert(&cmpd->compound.stmts, cmpd->compound.stmts.len, st_copy(st));
}


