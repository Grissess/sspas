#ifndef AST_H
#define AST_H

#include "type.h"
#include "vector.h"
#include "scope.h"
#include "lit.h"

typedef enum {
	EX_LIT,
	EX_ASSIGN,
	EX_INDEX,
	EX_SETINDEX,
	EX_CALL,
} expr_k;

typedef struct _expr_node expr_node;

typedef struct _lit_expr {
	literal *lit;
} lit_expr;

typedef struct _assign_expr {
	char *ident;
	expr_node *value;
} assign_expr;

typedef struct _index_expr {
	expr_node *object;
	expr_node *index;
} index_expr;

typedef struct _setindex_expr {
	expr_node *object;
	expr_node *index;
	expr_node *value;
} setindex_expr;

typedef struct _call_expr {
	expr_node *func;
	vector params; /* of expr_node * */
} call_expr;

typedef struct _expr_node{
	expr_k kind;
	type *type;
	size_t refcnt;
	union {
		lit_expr lit;
		assign_expr assign;
		index_expr index;
		setindex_expr setindex;
		call_expr call;
	};
} expr_node;

/*********************************************************************/

typedef enum {
	ST_EXPR,
    ST_WHILE,
	ST_IF,
	ST_FOR,
	ST_ITER,
	ST_BREAK,
	ST_CONTINUE,
	ST_COMPOUND,
} stmt_k;

typedef struct _stmt_node stmt_node;

typedef struct _expr_stmt {
	expr_node *expr;
} expr_stmt;

typedef struct _while_stmt {
	expr_node *cond;
	stmt_node *body;
} while_stmt;

typedef struct _if_stmt {
	expr_node *cond;
	stmt_node *iftrue;
	stmt_node *iffalse;
} if_stmt;

typedef struct _for_stmt {
	stmt_node *init;
	expr_node *cond;
	stmt_node *post;
	stmt_node *body;
} for_stmt;

typedef struct _iter_stmt {
	expr_node *value;
	char *ident;
	stmt_node *body;
} iter_stmt;

typedef struct _compound_stmt {
	vector stmts; /* of stmt_node * */
} compound_stmt;

typedef struct _stmt_node {
	stmt_k kind;
	size_t refcnt;
	union {
		expr_stmt expr;
		while_stmt while_;
		if_stmt if_;
		for_stmt for_;
		iter_stmt iter;
		compound_stmt compound;
	};
} stmt_node;

/*********************************************************************/

typedef struct _program program;

typedef struct _program {
	char *ident;
	vector args; /* of char * */
	scope scope;
} program;

#endif
