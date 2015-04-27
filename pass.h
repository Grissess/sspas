#ifndef PASS_H
#define PASS_H

#include "ast.h"
#include "sem.h"

typedef int (*pass_f)(ast_root *, object *);
typedef void (*pass_print_f)(int);

typedef struct _pass {
    pass_f run;
    pass_print_f print;
    char *name;
} pass;

extern pass passes[];

object *pass_do_all(ast_root *ast);

int stb_pass(ast_root *ast,object *obj);
int stb_visit_prog(prog_node *node,program *prog);
type *stb_resolve_type(type *ty,scope *sco);
type *stb_resolve_prog_type(prog_node *prog,scope *sco);
int stb_test_decl(program *prog,decl_node *decl,vector *decls,size_t idx);

int tr_pass(ast_root *, object *);
int tr_visit_prog(program *);
void tr_visit_stmt(stmt_node *, scope *);
void tr_visit_expr(expr_node *, scope *);

#endif
