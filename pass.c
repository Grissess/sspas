#include <stdarg.h>
#include <assert.h>

#include "pass.h"

#define ASSURE(x) ({int __test = (x); if(__test<0) return __test; __test;})

pass passes[] = {
	{stb_pass, NULL, "Syntax Tree Builder"},
    {tr_pass, NULL, "Type Resolution/Checking"},
};

object *pass_do_all(ast_root *ast) {
	size_t i;
	int res;
	object *obj = obj_new();
	for(i = 0; i < (sizeof(passes) / sizeof(*passes)); i++) {
		res = passes[i].run(ast, obj);
#ifndef NDEBUG
		fprintf(stderr, "\x1b[34;1m==== PASS %ld (%s) =====\x1b[m\n", i, passes[i].name);
		prog_print(stderr, 0, ast->prog);
		obj_print(stderr, 0, obj);
#endif
		if(res) {
			if(passes[i].print) {
				passes[i].print(res);
			} else {
				fprintf(stderr, "\x1b[37;41;1mPass %ld failed with code %d\x1b[m\n", i, res);
				exit(1);
			}
		}
	}
	return obj;
}

void pass_error(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
    pass_verror(fmt, va);
	va_end(va);
}

void pass_verror(const char *fmt, va_list va) {
	fputs("\x1b[37;41;1mERROR: ", stderr);
	vfprintf(stderr, fmt, va);
	fputs("\x1b[m\n", stderr);
	exit(1);
}

void pass_warning(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
    pass_vwarning(fmt, va);
	va_end(va);
}

void pass_vwarning(const char *fmt, va_list va) {
	fputs("\x1b[33;1mWarning: ", stderr);
	vfprintf(stderr, fmt, va);
	fputs("\x1b[m\n", stderr);
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
	ASSURE(vec_test(&node->decls, (vec_test_f) stb_test_decl, prog));
    return stb_test_stmt(prog, node->body);
}

type *stb_resolve_type(type *ty, scope *sco) {
	if(ty->kind == TP_REF) {
		symbol *res = scope_resolve_type(sco, ty->ref);
		if(!res) {
			pass_error("Unresolved type name: %s", ty->ref);
		}
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
			if(decl->init) {
				scope_add_name(prog->scope, sym_new_data_init(decl->ident, stb_resolve_type(decl->type, prog->scope), NULL, decl->init));
			} else {
				scope_add_name(prog->scope, sym_new_data(decl->ident, stb_resolve_type(decl->type, prog->scope), NULL));
			}
			break;

		case DECL_FUNC:
		case DECL_PROC:
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

int stb_test_stmt(program *prog, stmt_node *st) {
    size_t i;
    switch(st->kind) {
        case ST_ITER:
            scope_add_name(prog->scope, sym_new_data(st->iter.ident, type_new_int(), NULL));
            break;

        case ST_RANGE:
            scope_add_name(prog->scope, sym_new_data(st->range.ident, type_new_real(), NULL));
            break;

        case ST_COMPOUND:
            for(i = 0; i < st->compound.stmts.len; i++) {
                stb_test_stmt(prog, vec_get(&st->compound.stmts, i, stmt_node));
            }
            break;

        default:
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

void tr_check_cast(cast_k kind, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    if(kind <= CAST_EXPLICIT) {
        pass_verror(fmt, va);
    }
    if(kind <= CAST_UNINTENDED) {
        pass_vwarning(fmt, va);
    }
    va_end(va);
}

void tr_visit_stmt(stmt_node *st, scope *sco) {
	size_t i;
    symbol *sym;
	if(!st) {
		return;
	}
	switch(st->kind) {
		case ST_EXPR:
			tr_visit_expr(st->expr.expr, sco);
			break;

		case ST_WHILE:
			tr_visit_expr(st->while_.cond, sco);
			tr_visit_stmt(st->while_.body, sco);
            tr_check_cast(type_can_cast(st->while_.cond->type, type_new_bool()), "%s as while condition", type_repr(st->while_.cond->type));
			break;

		case ST_IF:
			tr_visit_expr(st->if_.cond, sco);
			tr_visit_stmt(st->if_.iftrue, sco);
			tr_visit_stmt(st->if_.iffalse, sco);
            tr_check_cast(type_can_cast(st->if_.cond->type, type_new_bool()), "%s as if condition", type_repr(st->if_.cond->type));
			break;

		case ST_FOR:
			tr_visit_stmt(st->for_.init, sco);
			tr_visit_expr(st->for_.cond, sco);
			tr_visit_stmt(st->for_.post, sco);
			tr_visit_stmt(st->for_.body, sco);
            tr_check_cast(type_can_cast(st->for_.cond->type, type_new_bool()), "%s as for condition", type_repr(st->for_.cond->type));
			break;

		case ST_ITER:
			tr_visit_expr(st->iter.value, sco);
			tr_visit_stmt(st->iter.body, sco);
            tr_check_cast(type_can_iter(st->iter.value->type), "Iter over %s", type_repr(st->iter.value->type));
            sym = scope_resolve_name(sco, st->iter.ident);
            if(!sym) pass_error("Unknown symbol %s", st->iter.ident);
            tr_check_cast(type_can_cast(sym->type, type_new_int()), "Iter using %s variable", type_repr(sym->type));
			break;

		case ST_RANGE:
			tr_visit_expr(st->range.lbound, sco);
			tr_visit_expr(st->range.ubound, sco);
			tr_visit_expr(st->range.step, sco);
			tr_visit_stmt(st->range.body, sco);
            tr_check_cast(type_can_cast(st->range.lbound->type, type_new_real()), "%s as lower range bound", type_repr(st->range.lbound->type));
            tr_check_cast(type_can_cast(st->range.ubound->type, type_new_real()), "%s as upper range bound", type_repr(st->range.ubound->type));
            tr_check_cast(type_can_cast(st->range.step->type, type_new_real()), "%s as range step", type_repr(st->range.step->type));
            sym = scope_resolve_name(sco, st->range.ident);
            if(!sym) pass_error("Unknown symbol %s", st->range.ident);
            tr_check_cast(type_can_cast(sym->type, type_new_real()), "Range using %s variable", type_repr(sym->type));
			break;

		case ST_COMPOUND:
			for(i = 0; i < st->compound.stmts.len; i++) {
				tr_visit_stmt(vec_get(&st->compound.stmts, i, stmt_node), sco);
			}
			break;

		default:
			assert(0);
	}
}

void tr_visit_expr(expr_node *ex, scope *sco) {
	size_t i;
	symbol *sym;
    vector ptypes;
	if(!ex) {
		return;
	}
	switch(ex->kind) {
		case EX_LIT:
			ex->type = ex->lit.lit->type;
			break;

		case EX_REF:
			sym = scope_resolve_name(sco, ex->ref.ident);
			if(!sym) {
				pass_error("Unknown symbol %s", ex->ref.ident);
			}
			ex->type = sym->type;
			break;

		case EX_ASSIGN:
			tr_visit_expr(ex->assign.value, sco);
            sym = scope_resolve_name(sco, ex->assign.ident);
            if(!sym) {
                pass_error("Unknown symbol %s", ex->assign.ident);
            }
            tr_check_cast(type_can_cast(ex->assign.value->type, sym->type), "Assign %s to var %s of type %s", type_repr(ex->assign.value->type), ex->assign.ident, type_repr(sym->type));
			ex->type = ex->assign.value->type;
			break;

		case EX_INDEX:
			tr_visit_expr(ex->index.object, sco);
			tr_visit_expr(ex->index.index, sco);
            tr_check_cast(type_can_index(ex->index.object->type, ex->index.index->type), "Index %s by %s", type_repr(ex->index.object->type), type_repr(ex->index.index->type));
            ex->type = type_of_index(ex->index.object->type, ex->index.index->type);
			break;

		case EX_SETINDEX:
			tr_visit_expr(ex->setindex.object, sco);
			tr_visit_expr(ex->setindex.index, sco);
			tr_visit_expr(ex->setindex.value, sco);
            tr_check_cast(type_can_setindex(ex->setindex.object->type, ex->setindex.index->type, ex->setindex.value->type), "Set index of %s by %s to %s", type_repr(ex->setindex.object->type), type_repr(ex->setindex.index->type), type_repr(ex->setindex.value->type));
            ex->type = ex->setindex.value->type;
			break;

		case EX_CALL:
			tr_visit_expr(ex->call.func, sco);
            vec_init(&ptypes);
			for(i = 0; i < ex->call.params.len; i++) {
				tr_visit_expr(vec_get(&ex->call.params, i, expr_node), sco);
                vec_insert(&ptypes, ptypes.len, vec_get(&ex->call.params, i, expr_node)->type);
			}
            tr_check_cast(type_can_call(ex->call.func->type, &ptypes), "Call %s with args %s", type_repr(ex->call.func->type), type_repr(type_new_func(NULL, &ptypes)));
            ex->type = type_of_call(ex->call.func->type, &ptypes);
			break;

		case EX_UNOP:
			tr_visit_expr(ex->unop.expr, sco);
            tr_check_cast(type_can_unop(ex->unop.expr->type, ex->unop.kind), "Unop %d on %s", ex->unop.kind, type_repr(ex->unop.expr->type));
            ex->type = type_of_unop(ex->unop.expr->type, ex->unop.kind);
			break;

		case EX_BINOP:
			tr_visit_expr(ex->binop.left, sco);
			tr_visit_expr(ex->binop.right, sco);
            tr_check_cast(type_can_binop(ex->binop.left->type, ex->binop.kind, ex->binop.right->type), "Binop %d on %s and %s", ex->binop.kind, type_repr(ex->binop.left->type), type_repr(ex->binop.right->type));
            ex->type = type_of_binop(ex->binop.left->type, ex->binop.kind, ex->binop.right->type);
			break;

		default:
			assert(0);
	}
}
