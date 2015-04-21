#ifndef SCOPE_H
#define SCOPE_H

#include "type.h"
#include "vector.h"
#include "loc.h"
#ifndef AST_H
#include "ast.h"
#endif

typedef struct _symbol {
	char *ident;
	type *type;
	location *loc;
	union {
		expr_node *expr;
		program *prog;
		void *ptr;
	} init;
} symbol;

typedef struct _scope {
	struct _scope *parent;
	vector children; /* of scope * */
	vector syms; /* of symbol * */
} scope;

#endif
