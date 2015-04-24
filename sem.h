#ifndef SEM_H
#define SEM_H

#include "type.h"
#include "vector.h"
#include "loc.h"
#include "ast.h"

typedef struct _scope scope;

typedef struct _program {
	size_t refcnt;
	char *ident;
	vector args; /* of char * */
	scope *scope;
	stmt_node *body;
} program;

typedef enum {
	SYM_PROG,
	SYM_DATA,
} symbol_k;

typedef struct _symbol {
	symbol_k kind;
	size_t refcnt;
	char *ident;
	type *type;
	location *loc;
	union {
		expr_node *expr;
		program *prog;
	} init;
} symbol;

typedef struct _scope {
	size_t refcnt;
	struct _scope *parent;
	vector children; /* of scope * */
	vector syms; /* of symbol * */
} scope;

typedef struct _object {
	program *root_prot;
} object;

#endif
