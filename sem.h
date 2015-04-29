#ifndef SEM_H
#define SEM_H

#include "type.h"
#include "vector.h"
#include "loc.h"
#include "ast.h"

typedef struct _scope scope;

typedef struct _program {
	size_t refcnt;
	scope *scope;
	prog_node *node;
} program;

program *program_new(prog_node *node, scope *scope);
program *program_copy(program *prog);
void program_delete(program *prog);
void program_destroy(program *prog);
void program_print(FILE *, int, program *);

typedef enum {
	SYM_PROG,
	SYM_DATA,
	SYM_TYPE,
} symbol_k;

typedef struct _symbol {
	symbol_k kind;
	size_t refcnt;
	char *ident;
	type *type;
	scope *scope;
	location *loc;
	union {
		expr_node *expr;
		program *prog;
	} init;
} symbol;

symbol *sym_new_data(const char *ident, type *type, location *loc);
symbol *sym_new_data_init(const char *ident, type *type, location *loc, expr_node *expr);
symbol *sym_new_prog(const char *ident, type *type, location *loc, program *prog);
symbol *sym_new_type(const char *ident, type *type);
symbol *sym_copy(symbol *sym);
void sym_delete(symbol *sym);
void sym_destroy(symbol *sym);
void sym_print(FILE *, int, symbol *);

typedef struct _scope {
	size_t refcnt;
	struct _scope *parent;
	vector children; /* of scope * */
	vector names; /* of symbol * */
	vector types; /* of symbol * */
	program *prog;
} scope;

scope *scope_new_root(void);
scope *scope_new(scope *parent);
scope *scope_new_above(scope *child);
int _scope_test_sym_name(const char *name, symbol *sym, vector *names, size_t idx);
symbol *scope_resolve_name(scope *sco, const char *name);
symbol *scope_resolve_type(scope *sco, const char *name);
void scope_add_name(scope *sco, symbol *sym);
void scope_add_type(scope *sco, symbol *sym);
scope *scope_copy(scope *sco);
void scope_delete(scope *sco);
void scope_destroy(scope *sco);
void scope_print(FILE *, int, scope *);

typedef struct _object {
	program *root_prog;
} object;

object *obj_new(void);
void obj_set_root_prog(object *obj, program *root_prog);
void obj_delete(object *obj);
void obj_print(FILE *, int, object *);

#endif
