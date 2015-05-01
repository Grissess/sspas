#ifndef PASS_H
#define PASS_H

#include "ast.h"
#include "sem.h"
#include "cg.h"

typedef int (*pass_f)(ast_root *, object *);
typedef void (*pass_print_f)(int);

typedef struct _pass {
	pass_f run;
	pass_print_f print;
	char *name;
} pass;

extern pass passes[];

object *pass_do_all(ast_root *ast);
void pass_error(const char *fmt,...);
void pass_verror(const char *fmt,va_list va);
void pass_warning(const char *fmt,...);
void pass_vwarning(const char *fmt,va_list va);

int stb_pass(ast_root *ast, object *obj);
int stb_visit_prog(prog_node *node, program *prog);
type *stb_resolve_type(type *ty, scope *sco);
type *stb_resolve_prog_type(prog_node *prog, scope *sco);
int stb_test_decl(program *prog, decl_node *decl, vector *decls, size_t idx);
int stb_test_stmt(program *prog, stmt_node *st);

int tr_pass(ast_root *, object *);
int tr_visit_prog(program *);
void tr_visit_stmt(stmt_node *, scope *);
void tr_visit_expr(expr_node *, scope *);

int lr_pass(ast_root *, object *);
void lr_visit_prog(program *, size_t *);
location *lr_calc_gdentry(size_t idx);

typedef struct _ir_ev_res {
	block *block;
	location *loc;
} ir_ev_res;

int ir_pass(ast_root *ast,object *obj);
block *ir_visit_prog(program *prog,block *superblk);
block *ir_make_prologue(program *prog,block *pblk);
block *ir_make_epilogue(program *prog,block *pblk);
block *ir_visit_stmt(stmt_node *st,block *pblk,scope *sco);
ir_ev_res ir_visit_expr(expr_node *ex,block *pblk,scope *sco);
#endif
