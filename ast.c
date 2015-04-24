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

expr_node *ex_new_unop(unop_k kind, expr_node *expr) {
	expr_node *res = ex_new();
	res->kind = EX_UNOP;
	res->unop.kind = kind;
	res->unop.expr = ex_copy(expr);
	return res;
}

expr_node *ex_new_binop(expr_node *left, binop_k kind, expr_node *right) {
	expr_node *res = ex_new();
	res->kind = EX_BINOP;
	res->binop.left = ex_copy(left);
	res->binop.kind = kind;
	res->binop.right = ex_copy(right);
	return res;
}

void ex_delete(expr_node *ex) {
	if(!(--ex->refcnt)) {
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

		case EX_UNOP:
			ex_delete(ex->unop.expr);
			break;

		case EX_BINOP:
			ex_delete(ex->binop.left);
			ex_delete(ex->binop.right);
			break;

		default:
			assert(0);
			break;
	}
	if(ex->type) type_delete(ex->type);
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
	if(iffalse) res->if_.iffalse = st_copy(iffalse);
	else res->if_.iffalse = NULL;
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
	assert(res);
	res->kind = ST_COMPOUND;
	vec_map(stmts, &res->compound.stmts, (vec_map_f) st_copy, NULL);
	return res;
}

void st_compound_append(stmt_node *cmpd, stmt_node *st) {
	vec_insert(&cmpd->compound.stmts, cmpd->compound.stmts.len, st_copy(st));
}

void st_delete(stmt_node *st) {
	if(!(--st->refcnt)) {
		st_destroy(st);
	}
}

void st_destroy(stmt_node *st) {
	switch(st->kind) {
		case ST_EXPR:
			ex_delete(st->expr.expr);
			break;

		case ST_WHILE:
			ex_delete(st->while_.cond);
			st_delete(st->while_.body);
			break;

		case ST_IF:
			ex_delete(st->if_.cond);
			st_delete(st->if_.iftrue);
			st_delete(st->if_.iffalse);
			break;

		case ST_FOR:
			st_delete(st->for_.init);
			ex_delete(st->for_.cond);
			st_delete(st->for_.post);
			st_delete(st->for_.body);
			break;

		case ST_ITER:
			ex_delete(st->iter.value);
			free(st->iter.ident);
			st_delete(st->iter.body);
			break;

		case ST_COMPOUND:
			vec_foreach(&st->compound.stmts, st_delete, NULL);
			vec_clear(&st->compound.stmts);
			break;

		default:
			assert(0);
	}
	free(st);
}

decl_node *decl_new(const char *ident, type *ty) {
	decl_node *res = malloc(sizeof(decl_node));
	assert(res);
	res->ident = strdup(ident);
	res->type = type_copy(ty);
	res->init = NULL;
	return ret;
}

decl_node *decl_new_init(const char *ident, type *ty, expr_node *init) {
	decl_node *res = decl_new(ident, ty);
	res->init = ex_copy(init);
	res->kind = DECL_VAR;
	return res;
}

decl_node *decl_new_func(const char *ident, type *ty, prog_node *prog) {
	decl_node *res = decl_new(ident, ty);
	res->prog = prog_copy(prog);
	res->kind = DECL_FUNC;
	return res;
}

decl_node *decl_new_proc(const char *ident, type *ty, prog_node *prog) {
	decl_node *res = decl_new(ident, ty);
	res->prog = prog_copy(prog);
	res->kind = DECL_PROC;
	return res;
}

decl_node *decl_copy(decl_node *decl) {
	return decl;
}

void decl_delete(decl_node *decl) {
	decl_destroy(decl);
}

void decl_destroy(decl_node *decl) {
	switch(decl->kind) {
		case DECL_FUNC: case DECL_PROC:
			prog_delete(decl->prog);
			break;

		case DECL_VAR:
			if(decl->init) ex_delete(decl->init);
			break;

		default:
			assert(0);
	}
	free(decl->ident);
	type_delete(decl->type);
	free(decl);
}

prog_node *prog_new(const char *ident, vector *args, vector *decls, stmt_node *body) {
	prog_node *res = malloc(sizeof(prog_node));
	res->ident = strdup(ident);
	vec_init(&res->args);
	vec_init(&res->decls);
	vec_map(args, &res->args, (vec_map_f) decl_copy, NULL);
	vec_map(decls, &res->decls, (vec_map_f) decl_copy, NULL);
	res->body = st_copy(body);
	return res;
}

prog_node *prog_copy(prog_node *prog) {
	return prog;
}

void prog_delete(prog_node *prog) {
	prog_destroy(prog);
}

void prog_destroy(prog_node *prog) {
	free(prog->ident);
	vec_foreach(&res->args, (vec_iter_f) decl_delete, NULL);
	vec_foreach(&res->decls, (vec_iter_f) decl_delete, NULL);
	vec_clear(&res->args);
	vec_clear(&res->decls);
	st_delete(res->body);
	free(prog);
}
