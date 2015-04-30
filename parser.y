%include {
#include <stdio.h>
#include <assert.h>

#include "ast.h"
#include "vector.h"
#include "lit.h"

#define NEW(ty) (malloc(sizeof(ty)))
#define AS(ty, ex) ((ty *) (ex))
}

%token_prefix TOK_

%token_type {void *}

%extra_argument {ast_root *ast}



object ::= PROGRAM IDENT(ident) argument_decl(args) SEMICOLON declarations(decls) compound_stmt(body) DOT. {
	ast->prog = prog_new(ident, args, decls, NULL, body);
}

argument_decl(ret) ::= LPAREN argument_list(args) RPAREN. {
	ret = args;
}

argument_list(ret) ::= argument_list(args) argument(arg). {
	vec_insert(args, AS(vector, args)->len, arg);
	ret = args;
}
argument_list(ret) ::= argument_list(args) SEMICOLON argument(arg). {
	vec_insert(args, AS(vector, args)->len, arg);
	ret = args;
}
argument_list(ret) ::= argument_list(args) COMMA argument(arg). {
	vec_insert(args, AS(vector, args)->len, arg);
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
	ret = decl_new(ident, type_new_int());
}

declarations(ret) ::= declarations(decls) declaration(decl_set). {
	vec_append(decl_set, decls);
	ret = decls;
}
declarations(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

declaration(ret) ::= VAR ident_list(idents) COLON type(ty) SEMICOLON. {
	ret = NEW(vector);
	vec_init(ret);
	vec_map(idents, ret, (vec_map_f) decl_new, ty);
}
declaration(ret) ::= VAR ident_list(idents) COLON type(ty) ASSIGN expr(init) SEMICOLON. {
    size_t i;
	ret = NEW(vector);
	vec_init(ret);
	for(i = 0; i < AS(vector, idents)->len; i++) vec_insert(ret, i, decl_new_init(vec_get(AS(vector, idents), i, char), ty, init));
}
declaration(ret) ::= FUNCTION IDENT(ident) argument_decl(args) COLON type(retty) SEMICOLON declarations(decls) compound_stmt(body) SEMICOLON. {
	ret = NEW(vector);
	vec_init(ret);
	vec_insert(ret, 0, decl_new_func(ident, NULL, prog_new(ident, args, decls, retty, body)));
}
declaration(ret) ::= PROCEDURE IDENT(ident) argument_decl(args) SEMICOLON declarations(decls) compound_stmt(body) SEMICOLON. {
	ret = NEW(vector);
	vec_init(ret);
	vec_insert(ret, 0, decl_new_proc(ident, NULL, prog_new(ident, args, decls, NULL, body)));
}
declaration(ret) ::= TYPE IDENT(ident) ASSIGN type(ty) SEMICOLON. {
	ret = NEW(vector);
	vec_init(ret);
	vec_insert(ret, 0, decl_new_type(ident, ty));
}

ident_list(ret) ::= ident_list(idents) IDENT(ident). {
	vec_insert(idents, AS(vector, idents)->len, ident);
	ret = idents;
}
ident_list(ret) ::= ident_list(idents) COMMA IDENT(ident). {
	vec_insert(idents, AS(vector, idents)->len, ident);
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
type(ret) ::= ARRAY LBRACKET LIT_INTEGER(lbound) DOTDOT LIT_INTEGER(ubound) RBRACKET OF type(base). {
	ret = type_new_array(base, *AS(long, lbound), *AS(long, ubound) - *AS(long, lbound));
}
type(ret) ::= ARRAY LBRACKET LIT_INTEGER(lbound) DOTDOT RBRACKET OF type(base). {
	ret = type_new_array(base, *AS(long, lbound), -1);
}
type(ret) ::= LPAREN type_list(args) RPAREN ARROW type(retty). {
	ret = type_new_func(retty, args);
}
type(ret) ::= IDENT(ref). {
	ret = type_new_ref(ref);
}

type_list(ret) ::= type_list(types) type(ty). {
	vec_insert(types, AS(vector, types)->len, type_copy(ty));
	ret = types;
}
type_list(ret) ::= type_list(types) COMMA type(ty). {
	vec_insert(types, AS(vector, types)->len, type_copy(ty));
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
stmt(ret) ::= range_stmt(stmt). {
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

for_stmt(ret) ::= FOR LPAREN stmt(init) SEMICOLON expr(cond) SEMICOLON stmt(post) LPAREN DO stmt(body). {
	ret = st_new_for(init, cond, post, body);
}

iter_stmt(ret) ::= FOR LPAREN IDENT(ident) IN expr(value) RPAREN DO stmt(body). {
	ret = st_new_iter(value, ident, body);
}
iter_stmt(ret) ::= FOR IDENT(ident) IN expr(value) DO stmt(body). {
	ret = st_new_iter(value, ident, body);
}

range_stmt(ret) ::= FOR LPAREN IDENT(ident) ASSIGN expr(lbound) dotdot_or_to expr(ubound) RPAREN DO stmt(body). {
	ret = st_new_range(ident, lbound, ubound, ex_new_lit(lit_new_real(1.0)), body);
}
range_stmt(ret) ::= FOR IDENT(ident) ASSIGN expr(lbound) dotdot_or_to expr(ubound) DO stmt(body). {
	ret = st_new_range(ident, lbound, ubound, ex_new_lit(lit_new_real(1.0)), body);
}

compound_stmt(ret) ::= BEGIN stmt_list(stmts) END. {
	ret = st_new_compound(stmts);
}

stmt_list(ret) ::= stmt_list(stmts) stmt(stmt). {
	vec_insert(stmts, AS(vector, stmts)->len, st_copy(stmt));
	ret = stmts;
}
stmt_list(ret) ::= stmt_list(stmts) SEMICOLON stmt(stmt). {
	vec_insert(stmts, AS(vector, stmts)->len, st_copy(stmt));
	ret = stmts;
}
stmt_list(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

expr(ret) ::= assign_expr(expr). {
	ret = expr;
}

expr_list(ret) ::= expr_list(exprs) expr(expr). {
	vec_insert(exprs, AS(vector, exprs)->len, ex_copy(expr));
	ret = exprs;
}
expr_list(ret) ::= expr_list(exprs) COMMA expr(expr). {
	vec_insert(exprs, AS(vector, exprs)->len, ex_copy(expr));
	ret = exprs;
}
expr_list(ret) ::= . {
	ret = NEW(vector);
	vec_init(ret);
}

assign_expr(ret) ::= IDENT(ident) ASSIGN assign_expr(expr). {
	ret = ex_new_assign(ident, expr);
}
assign_expr(ret) ::= index_expr(expr_index) ASSIGN assign_expr(value). {
	ret = ex_new_setindex(AS(expr_node, expr_index)->index.object, AS(expr_node, expr_index)->index.index, value);
}
assign_expr(ret) ::= logic_or_expr(expr). {
	ret = expr;
}

logic_or_expr(ret) ::= logic_or_expr(left) OR logic_and_expr(right). {
	ret = ex_new_binop(left, OP_OR, right);
}
logic_or_expr(ret) ::= logic_and_expr(expr). {
	ret = expr;
}

logic_and_expr(ret) ::= logic_and_expr(left) AND logic_unop_expr(right). {
	ret = ex_new_binop(left, OP_AND, right);
}
logic_and_expr(ret) ::= logic_unop_expr(expr). {
	ret = expr;
}

logic_unop_expr(ret) ::= NOT logic_unop_expr(expr). {
	ret = ex_new_unop(OP_NOT, expr);
}
logic_unop_expr(ret) ::= rel_expr(expr). {
	ret = expr;
}

rel_expr(ret) ::= rel_expr(left) EQ term_expr(right). {
	ret = ex_new_binop(left, OP_EQ, right);
}
rel_expr(ret) ::= rel_expr(left) NEQ term_expr(right). {
	ret = ex_new_binop(left, OP_NEQ, right);
}
rel_expr(ret) ::= rel_expr(left) LEQ term_expr(right). {
	ret = ex_new_binop(left, OP_LEQ, right);
}
rel_expr(ret) ::= rel_expr(left) GEQ term_expr(right). {
	ret = ex_new_binop(left, OP_GEQ, right);
}
rel_expr(ret) ::= rel_expr(left) LESS term_expr(right). {
	ret = ex_new_binop(left, OP_LESS, right);
}
rel_expr(ret) ::= rel_expr(left) GREATER term_expr(right). {
	ret = ex_new_binop(left, OP_GREATER, right);
}
rel_expr(ret) ::= term_expr(expr). {
	ret = expr;
}

term_expr(ret) ::= term_expr(left) ADD factor_expr(right). {
	ret = ex_new_binop(left, OP_ADD, right);
}
term_expr(ret) ::= term_expr(left) SUB factor_expr(right). {
	ret = ex_new_binop(left, OP_SUB, right);
}
term_expr(ret) ::= factor_expr(expr). {
	ret = expr;
}

factor_expr(ret) ::= factor_expr(left) MUL bin_or_expr(right). {
	ret = ex_new_binop(left, OP_MUL, right);
}
factor_expr(ret) ::= factor_expr(left) DIV bin_or_expr(right). {
	ret = ex_new_binop(left, OP_DIV, right);
}
factor_expr(ret) ::= factor_expr(left) MOD bin_or_expr(right). {
	ret = ex_new_binop(left, OP_MOD, right);
}
factor_expr(ret) ::= bin_or_expr(expr). {
	ret = expr;
}

bin_or_expr(ret) ::= bin_or_expr(left) BOR bin_and_expr(right). {
	ret = ex_new_binop(left, OP_BOR, right);
}
bin_or_expr(ret) ::= bin_and_expr(expr). {
	ret = expr;
}

bin_and_expr(ret) ::= bin_and_expr(left) BAND bin_xor_expr(right). {
	ret = ex_new_binop(left, OP_BAND, right);
}
bin_and_expr(ret) ::= bin_xor_expr(expr). {
	ret = expr;
}

bin_xor_expr(ret) ::= bin_xor_expr(left) BXOR bin_shift_expr(right). {
	ret = ex_new_binop(left, OP_BXOR, right);
}
bin_xor_expr(ret) ::= bin_shift_expr(expr). {
	ret = expr;
}

bin_shift_expr(ret) ::= bin_shift_expr(left) BLSHIFT num_unop_expr(right). {
	ret = ex_new_binop(left, OP_BLSHIFT, right);
}
bin_shift_expr(ret) ::= bin_shift_expr(left) BRSHIFT num_unop_expr(right). {
	ret = ex_new_binop(left, OP_BRSHIFT, right);
}
bin_shift_expr(ret) ::= num_unop_expr(expr). {
	ret = expr;
}

num_unop_expr(ret) ::= BNOT num_unop_expr(expr). {
	ret = ex_new_unop(OP_BNOT, expr);
}
num_unop_expr(ret) ::= SUB num_unop_expr(expr). {
	ret = ex_new_unop(OP_NEG, expr);
}
num_unop_expr(ret) ::= ADD num_unop_expr(expr). {
	ret = ex_new_unop(OP_IDENT, expr);
}
num_unop_expr(ret) ::= index_expr(expr). {
	ret = expr;
}

index_expr(ret) ::= index_expr(object) LBRACKET expr(index) RBRACKET. {
	ret = ex_new_index(object, index);
}
index_expr(ret) ::= call_expr(expr). {
	ret = expr;
}

call_expr(ret) ::= call_expr(func) LPAREN expr_list(params) RPAREN. {
	ret = ex_new_call(func, params);
}
call_expr(ret) ::= lit_expr(expr). {
	ret = expr;
}

lit_expr(ret) ::= LIT_INTEGER(ival). {
	ret = ex_new_lit(lit_new_int(*AS(long, ival)));
}
lit_expr(ret) ::= LIT_REAL(fval). {
	ret = ex_new_lit(lit_new_real(*AS(double, fval)));
}
lit_expr(ret) ::= LIT_CHAR(cval). {
	ret = ex_new_lit(lit_new_char(*AS(char, cval)));
}
lit_expr(ret) ::= LBRACE expr_list(init) RBRACE COLON type(fallback). {
	ret = ex_new_lit(lit_new_array(init, fallback));
}
lit_expr(ret) ::= LBRACE expr_list(init) RBRACE. {
	ret = ex_new_lit(lit_new_array(init, NULL));
}
lit_expr(ret) ::= ind_expr(expr). {
	ret = expr;
}

ind_expr(ret) ::= INDIRECT ind_expr(ind). {
	ret = ex_new_ind(ind);
}
ind_expr(ret) ::= ref_expr(expr). {
	ret = expr;
}

dotdot_or_to ::= DOTDOT.
dotdot_or_to ::= TO.

ref_expr(ret) ::= IDENT(ident). {
	ret = ex_new_ref(ident);
}
ref_expr(ret) ::= toplevel_expr(expr). {
	ret = expr;
}

toplevel_expr(ret) ::= LPAREN expr(expr) RPAREN. {
	ret = expr;
}



%parse_accept {
	fprintf(stderr, "Syntax check OK\n");
}

%parse_failure {
	fprintf(stderr, "Syntax check BAD\n");
}
