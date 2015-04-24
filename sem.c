#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "sem.h"
#include "type.h"
#include "loc.h"
#include "vector.h"

scope *scope_new_root(void) {
	scope *res = malloc(sizeof(scope));
	res->parent = NULL;
	res->refcnt = 1;
	vec_init(&res->children);
	vec_init(&res->syms);
	return res;
}

scope *scope_new(scope *parent) {
	scope *res = scope_new_root();
	res->parent = scope_copy(parent);
	vec_insert(&parent->children, 0, scope_copy(res));
	return res;
}

scope *scope_new_above(scope *child) {
	scope *res = scope_new_root();
	vec_insert(&res->children, 0, scope_copy(child));
	child->parent = scope_copy(res);


int _scope_test_sym_name(const char *name, symbol *sym, vector *syms, size_t idx) {
	if(!strcmp(name, sym->ident)) return idx+1;
	return 0;
}

symbol *scope_resolve_name(scope *sco, const char *name) {
	ssize_t idx = vec_test(&sco->syms, (vec_test_f) _scope_test_sym_name, (void *) name) - 1;
	if(idx < 0 || idx >= sco->syms.len) return NULL;
	return vec_get(&sco->syms, idx, symbol);
}

void scope_add_sym(scope *sco, symbol *sym) {
	ssize_t idx = vec_test(&sco->syms, (vec_test_f) _scope_test_sym_name, (void *) sym->ident) - 1;
	if(idx < 0 || idx >= sco->syms.len) {
		vec_insert(&sco->syms, 0, sym_copy(sym));
	} else {
		sym_delete(vec_get(&sco->syms, idx, symbol));
		vec_set(&sco->syms, idx, sym_copy(sym));
	}
}

scope *scope_copy(scope *sco) {
	sco->refcnt++;
	return sco;
}

void scope_delete(scope *sco) {
	if(!(--sco->refcnt)) {
		scope_destroy(sco);
	}
}

void scope_destroy(scope *sco) {
	vec_foreach(&sco->syms, sym_delete, NULL);
	vec_foreach(&sco->children, scope_delete, NULL);
	vec_remove(&sco->parent->children, vec_search(&sco->parent->children, sco));
	free(sco);
}

symbol *sym_new(const char *ident, type *type, location *loc) {
	symbol *res = malloc(sizeof(symbol));
	res->refcnt = 1;
	res->kind = SYM_DATA;
	res->ident = strdup(ident);
	res->type = type_copy(type);
	res->loc = loc_copy(loc);
	res->init.expr = NULL;
	return res;
}

symbol *sym_new_expr(const char *ident, type *type, location *loc, expr_node *expr) {
	symbol *res = sym_new(ident, type, loc);
	res->init.expr = ex_copy(expr);
	return res;
}

symbol *sym_new_prog(const char *ident, type *type, location *loc, program *prog) {
	symbol *res = sym_new(ident, type, loc);
	res->kind = SYM_PROG;
	res->init.prog = prog_copy(prog);
	return res;
}

symbol *sym_copy(symbol *sym) {
	sym->refcnt++;
	return sym;
}

void sym_delete(symbol *sym) {
	if(!(--sym->refcnt)) {
		sym_destroy(sym);
	}
}

void sym_destroy(symbol *sym) {
	switch(sym->kind) {
		case SYM_PROG:
			prog_delete(sym->init.prog);
			break;

		case SYM_DATA:
			if(sym->init.expr) ex_delete(sym->init.expr);
			break;

		default:
			assert(0);
	}
	free(sym);
}

program *prog_new(const char *ident, vector *args, stmt_node *body) {
	program *prog = malloc(sizeof(prog));
	prog->refcnt = 1;
	prog->ident = strdup(ident);
	vec_map(args, &prog->args, strdup, NULL);
	prog->body = st_copy(body);
	prog->scope = NULL;
	return prog;
}

program *prog_new_in(const char *ident, vector *args, program *super, stmt_node *body) {
	program *prog = prog_new(ident, args, body)
	prog->scope = scope_new(super->scope);
	return prog;
}

program *prog_new_root(const char *ident, vector *args, stmt_node *body) {
	program *prog = prog_new(ident, args, body);
	prog->scope = scope_new_root();
	return prog;
}

program *prog_copy(program *prog) {
	prog->refcnt++;
	return prog;
}

void prog_delete(program *prog) {
	if(!(--prog->refcnt)) {
		prog_destroy(prog);
	}
}

void prog_destroy(program *prog) {
	vec_foreach(&prog->args, free, NULL);
	vec_clear(&prog->args);
	scope_delete(prog->scope);
	free(prog);
}

object *obj_new(void) {
	object *res = malloc(sizeof(object));
	res->root_prog = NULL;
	return res;
}

void obj_set_root_prog(object *obj, program *root_prog) {
	if(obj->root_prog) prog_delete(obj->root_prog);
	obj->root_prog = prog_copy(root_prog);
}

void obj_delete(object *obj) {
	if(obj->root_prog) prog_delete(root_prog);
	free(obj);
}
