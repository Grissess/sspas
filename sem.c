#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "sem.h"
#include "type.h"
#include "loc.h"
#include "vector.h"
#include "util.h"

scope *scope_new_root(void) {
	scope *res = malloc(sizeof(scope));
	res->parent = NULL;
	res->refcnt = 1;
	vec_init(&res->children);
	vec_init(&res->names);
	vec_init(&res->types);
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
	return res;
}


int _scope_test_sym_name(const char *name, symbol *sym, vector *syms, size_t idx) {
	if(!strcmp(name, sym->ident)) return idx+1;
	return 0;
}

symbol *scope_resolve_name(scope *sco, const char *name) {
	ssize_t idx = vec_test(&sco->names, (vec_test_f) _scope_test_sym_name, (void *) name) - 1;
	if(idx < 0 || idx >= sco->names.len)
		if(sco->parent)
			return scope_resolve_name(sco->parent, name);
		else
			return NULL;
	return vec_get(&sco->names, idx, symbol);
}

symbol *scope_resolve_type(scope *sco, const char *name) {
	ssize_t idx = vec_test(&sco->types, (vec_test_f) _scope_test_sym_name, (void *) name) - 1;
	if(idx < 0 || idx >= sco->types.len)
		if(sco->parent)
			return scope_resolve_type(sco->parent, name);
		else
			return NULL;
	return vec_get(&sco->types, idx, symbol);
}

void scope_add_name(scope *sco, symbol *sym) {
	assert(sym->kind != SYM_TYPE);
	ssize_t idx = vec_test(&sco->names, (vec_test_f) _scope_test_sym_name, (void *) sym->ident) - 1;
	if(idx < 0 || idx >= sco->names.len) {
		vec_insert(&sco->names, 0, sym_copy(sym));
	} else {
		sym_delete(vec_get(&sco->names, idx, symbol));
		vec_set(&sco->names, idx, sym_copy(sym));
	}
}

void scope_add_type(scope *sco, symbol *sym) {
	assert(sym->kind == SYM_TYPE);
	ssize_t idx = vec_test(&sco->types, (vec_test_f) _scope_test_sym_name, (void *) sym->ident) - 1;
	if(idx < 0 || idx >= sco->types.len) {
		vec_insert(&sco->types, 0, sym_copy(sym));
	} else {
		sym_delete(vec_get(&sco->types, idx, symbol));
		vec_set(&sco->types, idx, sym_copy(sym));
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
	vec_foreach(&sco->names, (vec_iter_f) sym_delete, NULL);
	vec_foreach(&sco->children, (vec_iter_f) scope_delete, NULL);
	vec_remove(&sco->parent->children, vec_search(&sco->parent->children, sco));
	free(sco);
}

void scope_print(FILE *out, int lev, scope *sco) {
	size_t i;
	if(!sco) {
		wrlev(out, lev, "[(NULL)]");
		return;
	}
	wrlev(out, lev, "[SCOPE %p (IN %p)]", sco, sco->parent);
	wrlev(out, lev+1, "names:");
    for(i = 0; i < sco->names.len; i++) {
		sym_print(out, lev+2, vec_get(&sco->names, i, symbol));
	}
	wrlev(out, lev+1, "types:");
	for(i = 0; i < sco->types.len; i++) {
		sym_print(out, lev+2, vec_get(&sco->types, i, symbol));
	}
}

symbol *sym_new_data(const char *ident, type *type, location *loc) {
	symbol *res = malloc(sizeof(symbol));
	res->refcnt = 1;
	res->kind = SYM_DATA;
	res->ident = strdup(ident);
	res->type = type_copy(type);
	if(loc) res->loc = loc_copy(loc);
	else res->loc = NULL;
	res->init.expr = NULL;
	return res;
}

symbol *sym_new_data_init(const char *ident, type *type, location *loc, expr_node *expr) {
	symbol *res = sym_new_data(ident, type, loc);
	res->init.expr = ex_copy(expr);
	return res;
}

symbol *sym_new_prog(const char *ident, type *type, location *loc, program *prog) {
	symbol *res = sym_new_data(ident, type, loc);
	res->kind = SYM_PROG;
	res->init.prog = program_copy(prog);
	return res;
}

symbol *sym_new_type(const char *ident, type *type) {
	symbol *res = sym_new_data(ident, type, NULL);
	res->kind = SYM_TYPE;
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
			program_delete(sym->init.prog);
			break;

		case SYM_DATA:
			if(sym->init.expr) ex_delete(sym->init.expr);
			break;

		default:
			assert(0);
	}
	free(sym);
}

void sym_print(FILE *out, int lev, symbol *sym) {
	if(!sym) {
		wrlev(out, lev, "[(NULL)]");
		return;
	}
	wrlev(out, lev, "[SYM %s: %s]", sym->ident, type_repr(sym->type));
	if(sym->kind == SYM_PROG) {
		program_print(out, lev+1, sym->init.prog);
	}
}

program *program_new(prog_node *node, scope *scope) {
	program *prog = malloc(sizeof(program));
	prog->refcnt = 1;
	prog->node = prog_copy(node);
	prog->scope = scope;
	return prog;
}

program *program_copy(program *prog) {
	prog->refcnt++;
	return prog;
}

void program_delete(program *prog) {
	if(!(--prog->refcnt)) {
		program_destroy(prog);
	}
}

void program_destroy(program *prog) {
	scope_delete(prog->scope);
	prog_delete(prog->node);
	free(prog);
}

void program_print(FILE *out, int lev, program *prog) {
	if(!prog) {
		wrlev(out, lev, "[(NULL)]");
		return;
	}
	wrlev(out, lev, "[PROGRAM: %s]", prog->node->ident);
	scope_print(out, lev+1, prog->scope);
}

object *obj_new(void) {
	object *res = malloc(sizeof(object));
	res->root_prog = NULL;
	return res;
}

void obj_set_root_prog(object *obj, program *root_prog) {
	if(obj->root_prog) program_delete(obj->root_prog);
	obj->root_prog = program_copy(root_prog);
}

void obj_delete(object *obj) {
	if(obj->root_prog) program_delete(obj->root_prog);
	free(obj);
}

void obj_print(FILE *out, int lev, object *obj) {
	if(!obj) {
		wrlev(out, lev, "[(NULL)]");
		return;
	}
	wrlev(out, lev, "[OBJECT:]");
	program_print(out, lev+1, obj->root_prog);
}
