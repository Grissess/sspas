#ifndef AST_H
#define AST_H

#include <stdio.h>

#include "type.h"
#include "vector.h"
#include "lit.h"

typedef enum {
	EX_LIT,
	EX_REF,
	EX_ASSIGN,
	EX_INDEX,
	EX_SETINDEX,
	EX_CALL,
	EX_UNOP,
	EX_BINOP,
	EX_RETURN,
	EX_IND,
} expr_k;

typedef struct _expr_node expr_node;

typedef struct _lit_expr {
	literal *lit;
} lit_expr;

typedef struct _ref_expr {
	char *ident;
} ref_expr;

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

typedef enum {
	OP_NEG,
	OP_NOT,
	OP_BNOT,
	OP_IDENT,
} unop_k;

typedef struct _unop_expr {
	unop_k kind;
	expr_node *expr;
} unop_expr;

typedef enum {
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_EQ,
	OP_NEQ,
	OP_LEQ,
	OP_GEQ,
	OP_LESS,
	OP_GREATER,
	OP_AND,
	OP_OR,
	OP_BAND,
	OP_BOR,
	OP_BXOR,
	OP_BLSHIFT,
	OP_BRSHIFT,
} binop_k;

typedef struct _binop_expr {
	binop_k kind;
	expr_node *left;
	expr_node *right;
} binop_expr;

typedef struct _return_expr {
	expr_node *value;
} return_expr;

typedef struct _ind_expr {
	expr_node *lvalue;
} ind_expr;

typedef struct _expr_node {
	expr_k kind;
	type *type;
	size_t refcnt;
	union {
		lit_expr lit;
		ref_expr ref;
		assign_expr assign;
		index_expr index;
		setindex_expr setindex;
		call_expr call;
		unop_expr unop;
		binop_expr binop;
		return_expr return_;
		ind_expr ind;
	};
} expr_node;

expr_node *ex_new(void);
expr_node *ex_copy(expr_node *ex);
expr_node *ex_new_lit(literal *lit);
expr_node *ex_new_ref(const char *ident);
expr_node *ex_new_assign(char *name, expr_node *value);
expr_node *ex_new_index(expr_node *object, expr_node *index);
expr_node *ex_new_setindex(expr_node *object, expr_node *index, expr_node *value);
expr_node *ex_new_call(expr_node *func, vector *params);
expr_node *ex_new_unop(unop_k kind, expr_node *expr);
expr_node *ex_new_binop(expr_node *left, binop_k kind, expr_node *right);
expr_node *ex_new_return(expr_node *);
expr_node *ex_new_ind(expr_node *);
void ex_delete(expr_node *ex);
void ex_destroy(expr_node *ex);
void ex_print(FILE *, int, expr_node *);

/*********************************************************************/

typedef enum {
	ST_EXPR,
	ST_WHILE,
	ST_IF,
	ST_FOR,
	ST_ITER,
	ST_RANGE,
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

typedef struct _range_stmt {
	char *ident;
	expr_node *lbound;
	expr_node *ubound;
	expr_node *step;
	stmt_node *body;
} range_stmt;

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
		range_stmt range;
		compound_stmt compound;
	};
} stmt_node;

stmt_node *st_new(void);
stmt_node *st_copy(stmt_node *st);
stmt_node *st_new_expr(expr_node *ex);
stmt_node *st_new_while(expr_node *cond, stmt_node *body);
stmt_node *st_new_if(expr_node *cond, stmt_node *iftrue, stmt_node *iffalse);
stmt_node *st_new_for(stmt_node *init, expr_node *cond, stmt_node *post, stmt_node *body);
stmt_node *st_new_iter(expr_node *value, char *ident, stmt_node *body);
stmt_node *st_new_range(char *ident, expr_node *lbound, expr_node *ubound, expr_node *step, stmt_node *body);
stmt_node *st_new_compound(vector *stmts);
void st_compound_append(stmt_node *cmpd, stmt_node *st);
void st_delete(stmt_node *st);
void st_destroy(stmt_node *st);
void st_print(FILE *, int, stmt_node *);

/*********************************************************************/

typedef struct _prog_node prog_node;

typedef enum {
	DECL_VAR,
	DECL_FUNC,
	DECL_PROC,
	DECL_TYPE,
} decl_k;

typedef struct _decl_node {
	char *ident;
	type *type;
	decl_k kind;
	union {
		expr_node *init;
		prog_node *prog;
	};
} decl_node;

typedef struct _prog_node {
	char *ident;
	vector args; /* of decl_node * */
	vector decls; /* of decl_node * */
	type *ret;
	stmt_node *body;
} prog_node;

typedef struct _ast_root {
	prog_node *prog;
} ast_root;

decl_node *decl_new(const char *ident, type *ty);
decl_node *decl_new_init(const char *ident, type *ty, expr_node *init);
decl_node *decl_new_func(const char *ident, type *ty, prog_node *prog);
decl_node *decl_new_proc(const char *ident, type *ty, prog_node *prog);
decl_node *decl_new_type(const char *ident, type *ty);
decl_node *decl_copy(decl_node *decl);
void decl_delete(decl_node *decl);
void decl_destroy(decl_node *decl);
void decl_print(FILE *, int, decl_node *);
prog_node *prog_new(const char *ident, vector *args, vector *decls, type *ret, stmt_node *body);
prog_node *prog_copy(prog_node *prog);
void prog_delete(prog_node *prog);
void prog_destroy(prog_node *prog);
void prog_print(FILE *, int, prog_node *);

#endif
