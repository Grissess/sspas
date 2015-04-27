#include <stdarg.h>
#include <assert.h>

#include "pass.h"

#define ASSURE(x) ({int __test = (x); if(__test<0) return __test; __test;})

pass passes[]={
    {stb_pass, NULL, "Syntax Tree Builder"},
};

object *pass_do_all(ast_root *ast) {
    size_t i;
    int res;
    object *obj = obj_new();
    for(i = 0; i < (sizeof(passes)/sizeof(*passes)); i++) {
        res = passes[i].run(ast, obj);
        if(res) {
            if(passes[i].print)
                passes[i].print(res);
            else
                fprintf(stderr, "Pass %ld failed with code %d\n", i, res);
            exit(1);
        }
#ifndef NDEBUG
        fprintf(stderr, "===== PASS %ld (%s) =====\n", i, passes[i].name);
        prog_print(stderr, 0, ast->prog);
        obj_print(stderr, 0, obj);
#endif
    }
    return obj;
}

void pass_error(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    exit(1);
}

/********** SEMANTIC TREE BUILDER **********/

int stb_pass(ast_root *ast, object *obj) {
    program *prog = program_new(ast->prog, scope_new_root());
    obj_set_root_prog(obj, prog);
    return stb_visit_prog(ast->prog, prog);
}

int stb_visit_prog(prog_node *node, program *prog) {
    /* scope_add_name(prog->scope, sym_new_prog(node->ident, stb_resolve_prog_type(node, prog->scope), NULL, prog)); */
    ASSURE(vec_test(&node->args, (vec_test_f) stb_test_decl, prog));
    return vec_test(&node->decls, (vec_test_f) stb_test_decl, prog);
}

type *stb_resolve_type(type *ty, scope *sco) {
    if(ty->kind == TP_REF) {
        symbol *res = scope_resolve_type(sco, ty->ref);
        if(!res) pass_error("Unresolved type name: %s", ty->ref);
        return res->type;
    }
    return ty;
}

type *stb_resolve_prog_type(prog_node *prog, scope *sco) {
    vector params;
    size_t i;
    vec_init(&params);
    for(i = 0; i < prog->args.len; i++) {
        vec_insert(&params, params.len, stb_resolve_type(vec_get(&prog->args, i, decl_node)->type, sco));
    }
    return type_new_func(prog->ret, &params);
}

int stb_test_decl(program *prog, decl_node *decl, vector *decls, size_t idx) {
    program *subprog = NULL;
    switch(decl->kind) {
        case DECL_VAR:
            if(decl->init)
                scope_add_name(prog->scope, sym_new_data_init(decl->ident, stb_resolve_type(decl->type, prog->scope), NULL, decl->init));
            else
                scope_add_name(prog->scope, sym_new_data(decl->ident, stb_resolve_type(decl->type, prog->scope), NULL));
            break;

        case DECL_FUNC: case DECL_PROC:
            decl->type = stb_resolve_prog_type(decl->prog, prog->scope);
            subprog = program_new(decl->prog, scope_new(prog->scope));
            scope_add_name(prog->scope, sym_new_prog(decl->ident, decl->type, NULL, subprog));
            stb_visit_prog(decl->prog, subprog);
            break;

        case DECL_TYPE:
            scope_add_type(prog->scope, sym_new_type(decl->ident, decl->type));
            break;
    }
    return 0;
}

/********** Type Resolution **********/

int tr_pass(ast_root *ast, object *obj) {
    return tr_visit_prog(obj->root_prog);
}

int tr_visit_prog(program *prog) {
    size_t i;
    tr_visit_stmt(prog->node->body, prog->scope);
    for(i = 0; i < prog->scope->names.len; i++) {
        if(vec_get(&prog->scope->names, i, symbol)->kind == SYM_PROG) {
            tr_visit_prog(vec_get(&prog->scope->names, i, symbol)->init.prog);
        }
    }
    return 0;
}

void tr_visit_stmt(stmt_node *st, scope *sco) {
    size_t i;
    if(!st) return;
    switch(st->kind) {
        case ST_EXPR:
            tr_visit_expr(st->expr.expr, sco);
            break;

        case ST_WHILE:
            tr_visit_expr(st->while_.cond, sco);
            tr_visit_stmt(st->while_.body, sco);
            break;

        case ST_IF:
            tr_visit_expr(st->if_.cond, sco);
            tr_visit_stmt(st->if_.iftrue, sco);
            tr_visit_stmt(st->if_.iffalse, sco);
            break;

        case ST_FOR:
            tr_visit_stmt(st->for_.init, sco);
            tr_visit_expr(st->for_.cond, sco);
            tr_visit_stmt(st->for_.post, sco);
            tr_visit_stmt(st->for_.body, sco);
            break;

        case ST_ITER:
            tr_visit_expr(st->iter.value, sco);
            tr_visit_stmt(st->iter.body, sco);
            break;

        case ST_RANGE:
            tr_visit_expr(st->range.lbound, sco);
            tr_visit_expr(st->range.ubound, sco);
            tr_visit_expr(st->range.step, sco);
            tr_visit_stmt(st->range.body, sco);
            break;

        case ST_COMPOUND:
            for(i = 0; i < st->compound.stmts.len; i++)
                tr_visit_stmt(vec_get(&st->compound.stmts, i, stmt_node), sco);
            break;

        default:
            assert(0);
    }
}

void tr_visit_expr(expr_node *ex, scope *sco) {
    if(!ex) return;
    switch(ex->kind) {
        case 0:
            break;
    }
}
