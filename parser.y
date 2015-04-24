%include {
#include <stdio.h>
#include <assert.h>

#include "sem.h"
#include "vector.h"

#define NEW(ty) (malloc(sizeof(ty)))
#define AS(ty, ex) ((ty *) (ex))
}

%token_prefix TOK_

%token_type {void *}

%extra_argument {ast_root *ast}



object ::= PROGRAM IDENT(ident) argument_decl(args) SEMICOLON declarations(decls) compound_stmt(body) DOT. {
	ast->prog = prog_new(ident, args, decls, body);
}

argument_decl(ret) ::= LPAREN argument_list(args) RPAREN. {
	ret = args;
}

argument_list(ret) ::= argument_list(args) argument(arg). {
	vec_insert(args, args->len, arg);
	ret = args;
}
argument_list(ret) ::= argument_list(args) SEMICOLON argument(arg). {
	vec_insert(args, args->len, arg);
	ret = args;
}
argument_list(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

argument(ret) ::= IDENT(ident) COLON type(ty). {
	ret = decl_new(ident, ty);
}
argument(ret) ::= IDENT(ident). {
	ret = decl_new(ident, NULL);
}

declarations(ret) ::= declarations(decls) declaration(decl_set). {
	vec_append(decl_set, decls);
	ret = decls;
}
declarations(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

declaration(ret) ::= VAR ident_list(idents) COLON type (ty) SEMICOLON. {
	ret = NEW(vector);
	vec_init(ret);
	vec_map(idents, ret, (vec_map_f) decl_new, ty);
	vec_clear(idents);
	free(idents);
}

ident_list(ret) ::= ident_list(idents) IDENT(ident). {
	vec_insert(idents, idents->len, ident);
	ret = idents;
}
ident_list(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

type(ret) ::= INTEGER. {
	ret = type_new_int();
}
type(ret) ::= REAL. {
	ret = type_new_real();
}
type(ret) ::= CHARACTER. {
	ret = type_new_char();
}
type(ret) ::= ARRAY LBRACKET INTEGER(lbound) DOTDOT INTEGER(ubound) RBRACKET OF type(base). {
	ret = type_new_array(base, *lbound, *ubound - *lbound);
}
type(ret) ::= ARRAY LBRACKET INTEGER(lbound) DOTDOT RBRACKET OF type(base). {
	ret = type_new_array(base, *lbound, -1);
}
type(ret) ::= LPAREN type_list(args) RPARENT ARROW type(retty). {
	ret = type_new_func(retty, args);
}

type_list(ret) ::= type_list(types) type(ty). {
	vec_insert(types, types->len, type_copy(ty));
	ret = types;
}
type_list(ret) ::= type_list(types) COMMA type(ty). {
	vec_insert(types, types->len, type_copy(ty));
	ret = types;
}
type_list(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

stmt(ret) ::= expr_stmt(stmt). {
	ret = stmt;
}
stmt(ret) ::= while_stmt(stmt). {
	ret = stmt;
}
stmt(ret) ::= if_stmt(stmt). {
	ret = stmt;
}
stmt(ret) ::= for_stmt(stmt). {
	ret = stmt;
}
stmt(ret) ::= iter_stmt(stmt). {
	ret = stmt;
}
stmt(ret) ::= compound_stmt(stmt). {
	ret = stmt;
}

expr_stmt(ret) ::= expr(expr). {
	ret = st_new_expr(expr);
}

while_stmt(ret) ::= WHILE expr(cond) DO stmt(body). {
	ret = st_new_while(cond, body);
}

if_stmt(ret) ::= IF expr(cond) THEN stmt(iftrue). {
	ret = st_new_if(cond, iftrue, NULL);
}
if_stmt(ret) ::= IF expr(cond) THEN stmt(iftrue) ELSE stmt(iffalse). {
	ret = st_new_if(cond, iftrue, iffalse);
}

for_stmt(ret) ::= FOR LPAREN stmt(init) SEMICOLON expr(cond) SEMICOLON stmt(post) LPAREN stmt(body). {
	ret = st_new_for(init, cond, post, body);
}

compound_stmt(ret) ::= BEGIN stmt_list(stmts) END. {
	ret = st_new_compound(stmts);
}

stmt_list(ret) ::= stmt_list(stmts) stmt(stmt). {
	vec_insert(stmts, stmts->len, st_copy(stmt));
	ret = stmts;
}
stmt_list(ret) ::= stmt_list(stmts) SEMICOLON stmt(stmt). {
	vec_insert(stmts, stmts->len, st_copy(stmt));
	ret = stmts;
}
stmt_list(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

expr(ret) ::= assign_expr(expr). {
	ret = expr;
}

assign_expr(ret) ::= IDENT(ident) ASSIGN logic_expr(expr). {
	ret = ex_new_assign(ident, expr);
}
assign_expr(ret) ::= logic_expr(object) LBRACKET expr(index) RBRACKET ASSIGN logic_expr(value). {
	ret = ex_new_setindex(



parse_accept {
	fprintf(stderr, "Syntax check OK\n");
}

%parse_failure {
	fprintf(stderr, "Syntax check BAD\n");
}
