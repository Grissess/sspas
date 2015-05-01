#include <stdarg.h>
#include <assert.h>

#include "pass.h"
#include "util.h"
#include "cg.h"

#define ASSURE(x) ({int __test = (x); if(__test<0) return __test; __test;})

pass passes[] = {
	{stb_pass, NULL, "Semantic Tree Builder"},
    {tr_pass, NULL, "Type Resolution/Checking"},
	{lr_pass, NULL, "Location Resolution"},
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
	symbol *res;
	size_t i;
	if(!ty) return NULL;
	switch(ty->kind) {
		case TP_REF:
			res = scope_resolve_type(sco, ty->ref);
			if(!res) {
				pass_error("Unresolved type name: %s", ty->ref);
			}
			return res->type;

		case TP_ARRAY:
			ty->base = stb_resolve_type(ty->base, sco);
			break;

		case TP_FUNC:
			ty->ret = stb_resolve_type(ty->ret, sco);
			for(i = 0; i < ty->args.len; i++) {
				vec_set(&ty->args, i, stb_resolve_type(vec_get(&ty->args, i, type), sco));
			}
			break;

		case TP_STRUCT: case TP_UNION:
			for(i = 0; i < ty->types.len; i++) {
				vec_set(&ty->types, i, stb_resolve_type(vec_get(&ty->types, i, type), sco));
			}
			break;

		default:
			break;
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
	return type_new_func(stb_resolve_type(prog->ret, sco), &params);
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
	symbol *sym = NULL;
    vector ptypes;
	expr_node *temp;
	scope *lsco;
	if(!ex) {
		return;
	}
	switch(ex->kind) {
		case EX_LIT:
			ex->type = stb_resolve_type(ex->lit.lit->type, sco);
			break;

		case EX_REF:
			sym = scope_resolve_name(sco, ex->ref.ident);
			if(!sym) {
				pass_error("Unknown symbol %s", ex->ref.ident);
			}
			ex->type = stb_resolve_type(sym->type, sco);
			break;

		case EX_ASSIGN:
			tr_visit_expr(ex->assign.value, sco);
			lsco = sco;
			while(lsco) {
				if(string_equal(ex->assign.ident, lsco->prog->node->ident)) {
					sym = scope_resolve_name(sco, lsco->prog->node->ident);
					if(!sym) {
						pass_error("Could not resolve function %s when identifying return (BUG)", lsco->prog->node->ident);
					}
					if(!sym->type->kind == TP_FUNC) {
						pass_error("Program scope for %s not a function type (instead %s) (BUG)", lsco->prog->node->ident, type_repr(sym->type));
					}
					if(type_can_cast(ex->assign.value->type, sym->type->ret) >= CAST_UNINTENDED) {
						temp = ex->assign.value;
						ex->kind = EX_RETURN;
						ex->return_.value = temp;
						ex->type = stb_resolve_type(temp->type, sco);
						return;
					}
				}
				lsco = lsco->parent;
			}
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

		case EX_IND:
			tr_visit_expr(ex->ind.lvalue, sco);
			ex->type = type_new_array(ex->ind.lvalue->type, 0, -1);
			break;

		default:
			assert(0);
	}
}

/********** Location Resolution **********/

int lr_pass(ast_root *ast, object *obj) {
	size_t gdidx = 0;
	lr_visit_prog(obj->root_prog, &gdidx);
	scope_add_name(obj->root_prog->scope, sym_new_data(SYNAME_GDISP, type_new_array(type_new_int(), 0, gdidx), loc_new_sym(SYNAME_GDISP)));
	return 0;
}

void lr_visit_prog(program *prog, size_t *gdidx) {
	size_t i;
	vector amts;
	location *gdbase;
	vec_init(&amts);
	prog->gdidx = (*gdidx)++;
	gdbase = loc_new_ind(lr_calc_gdentry(prog->gdidx));
	vec_insert(&amts, 0, loc_new_stride(loc_new_size(NULL), loc_new_mem(2))); /* For ret addr + pushed FP */
	for(i = 0; i < prog->node->args.len; i++) {
		symbol *sym = scope_resolve_name(prog->scope, vec_get(&prog->node->args, i, decl_node)->ident);
		if(!sym) {
			pass_error("Couldn't resolve argument %s (BUG)", vec_get(&prog->node->args, i, decl_node)->ident);
		}
		sym->loc = loc_new_off_vec(loc_new_reg(REG_FP), &amts);
		vec_insert(&amts, amts.len, loc_new_size(sym->type));
	}
	vec_clear(&amts);
	for(i = 0; i < prog->node->decls.len; i++) {
		decl_node *decl = vec_get(&prog->node->decls, i, decl_node);
		symbol *sym = scope_resolve_name(prog->scope, decl->ident);
		if(decl->kind == DECL_TYPE) continue;
		if(!sym) {
			pass_error("Couldn't resolve declaration %s (BUG)", decl->ident);
		}
		switch(sym->kind) {
			case SYM_PROG:
				sym->loc = loc_new_sym(sym->init.prog->node->ident);
				lr_visit_prog(sym->init.prog, gdidx);
				break;

			case SYM_DATA:
				vec_insert(&amts, amts.len, loc_new_stride(loc_new_size(sym->type), loc_new_mem(-1)));
				sym->loc = loc_new_off_vec(gdbase, &amts);
				break;

			case SYM_TYPE:
				break;

			default:
				assert(0);
				break;
		}
	}
}

location *lr_calc_gdentry(size_t idx) {
	return loc_new_off(loc_new_sym(SYNAME_GDISP), loc_new_stride(loc_new_mem(idx), loc_new_size(NULL)));
}

/********** Intermediate Representation Generation **********/

int ir_pass(ast_root *ast, object *obj) {
	block *root = block_new(NULL);
	block *main = ir_visit_prog(obj->root_prog, root);
	block_append(root, main);
	obj->block = block_copy(root);
	return 0;
}

block *ir_visit_prog(program *prog, block *superblk) {
	block *blk = block_new_program(superblk, prog);
	block *prologue = ir_make_prologue(prog, blk);
	block *body = ir_visit_stmt(prog->node->body, blk, prog->scope);
	block *epilogue = ir_make_epilogue(prog, blk);
	block_append(blk, prologue);
	block_append(blk, body);
	block_append(blk, epilogue);
	return blk;
}

block *ir_make_prologue(program *prog, block *pblk) {
	location *fralloc = loc_new_reg(REG_SP);
	location *gdentry = lr_calc_gdentry(prog->gdidx);
	block *blk = block_new(pblk);
	size_t i;
	for(i = 0; i < prog->node->decls.len; i++) {
		decl_node *decl = vec_get(&prog->node->decls, i, decl_node);
		symbol *sym = scope_resolve_name(prog->scope, decl->ident);
		if(decl->kind != DECL_VAR) continue;
		if(!sym) {
			pass_error("Couldn't resolve variable %s (BUG)", decl->ident);
		}
		fralloc = loc_new_off(fralloc, loc_new_stride(loc_new_size(sym->type), loc_new_mem(-1)));
	}
	block_emit(blk, instr_new_push(loc_new_reg(REG_FP)));
	block_emit(blk, instr_new_laddr(loc_new_reg(REG_FP), loc_new_reg(REG_SP)));
	block_emit(blk, instr_new_push(gdentry));
	block_emit(blk, instr_new_laddr(gdentry, loc_new_reg(REG_SP)));
	block_emit(blk, instr_new_laddr(loc_new_reg(REG_SP), fralloc));
	return blk;
}

block *ir_make_epilogue(program *prog, block *pblk) {
	block *blk = block_new(pblk);
	location *gdentry = lr_calc_gdentry(prog->gdidx);
	block_emit(blk, instr_new_laddr(loc_new_reg(REG_SP), gdentry));
	block_emit(blk, instr_new_pop(gdentry));
	block_emit(blk, instr_new_pop(loc_new_reg(REG_FP)));
	return blk;
}

block *ir_visit_stmt(stmt_node *st, block *pblk, scope *sco) {
	size_t i;
	block *blk = block_new(pblk), *a, *b, *c;
	ir_ev_res x, y, z;
	instr *la, *lb, *lc;
	location *ta, *tb, *tc, *td, *te;
	symbol *sa, *sb, *sc;
	switch(st->kind) {
		case ST_EXPR:
			x = ir_visit_expr(st->expr.expr, blk, sco);
			block_append(blk, x.block);
			break;

		case ST_WHILE:
			x = ir_visit_expr(st->while_.cond, blk, sco);
			la = instr_new_label(NULL);
			lb = instr_new_label(NULL);
			block_emit(blk, la);
			block_append(blk, x.block);
			ta = loc_new_temp(NULL);
			block_emit(blk, instr_new_unop(ta, OP_NOT, x.loc));
			block_emit(blk, instr_new_jumpif(lb, ta));
			a = ir_visit_stmt(st->while_.body, blk, sco);
			block_append(blk, a);
			block_emit(blk, lb);
			break;

		case ST_IF:
			x = ir_visit_expr(st->if_.cond, blk, sco);
			la = instr_new_label(NULL);
			block_append(x.block);
			ta = loc_new_temp(NULL);
			block_emit(blk, instr_new_unop(ta, OP_NOT, x.loc));
			block_emit(blk, instr_new_jumpif(la, ta));
			a = ir_visit_stmt(st->if_.iftrue, blk, sco);
			block_append(blk, a);
			block_emit(blk, la);
			if(st->if_.iffalse) {
				b = ir_visit_stmt(st->if_.iffalse, blk, sco);
				block_append(blk, b);
			}
			break;

		case ST_FOR:
			a = ir_visit_stmt(st->for_.init, blk, sco);
			block_append(blk, a);
			la = instr_new_label(NULL);
			lb = instr_new_label(NULL);
			block_emit(blk, lb);
			x = ir_visit_expr(st->for_.cond, blk, sco);
			block_append(blk, x.block);
			ta = loc_new_temp(NULL);
			block_emit(blk, instr_new_unop(ta, OP_NOT, x.loc));
			block_emit(blk, instr_new_jumpif(la, ta));
			b = ir_visit_stmt(st->for_.body, blk, sco);
			block_append(blk, b);
			c = ir_visit_stmt(st->for_.post, blk, sco);
			block_append(blk, c);
			block_emit(blk, instr_new_jump(lb));
			block_emit(blk, la);
			break;

		case ST_ITER:
			x = ir_visit_expr(st->iter.value, blk, sco);
			block_append(blk, x.block);
			ta = loc_new_temp(NULL);
			tb = loc_new_temp(NULL);
			tc = loc_new_temp(NULL);
			block_emit(blk, instr_new_laddr(ta, loc_new_mem(st->iter.value->type->lbound)));
			block_emit(blk, instr_new_laddr(tb, loc_new_mem(st->iter.value->type->lbound + st->iter.value->type->size)));
			la = instr_new_label(NULL);
			lb = instr_new_label(NULL);
			block_emit(blk, la);
			block_emit(blk, instr_new_binop(tc, ta, OP_GEQ, tb));
			block_emit(blk, instr_new_jumpif(lb, tc));
			sa = scope_resolve_name(sco, st->iter.ident);
			block_emit(blk, instr_new_set(sa->loc, ta));
			a = ir_visit_stmt(st->iter.body, blk, sco);
			block_append(blk, a);
			block_emit(blk, instr_new_laddr(ta, loc_new_off(ta, loc_new_mem(1))));
			block_emit(blk, instr_new_jump(la));
			block_emit(blk, lb);
			break;

		case ST_RANGE:
			x = ir_visit_expr(st->range.lbound, blk, sco);
			y = ir_visit_expr(st->range.ubound, blk, sco);
			z = ir_visit_expr(st->range.step, blk, sco);
			block_append(blk, x.block);
			block_append(blk, y.block);
			block_append(blk, z.block);
			ta = loc_new_temp(NULL);
			tb = loc_new_temp(NULL);
			tc = loc_new_temp(NULL);
			td = loc_new_temp(NULL);
			block_emit(blk, instr_new_set(ta, x.loc));
			block_emit(blk, instr_new_set(tb, y.loc));
			block_emit(blk, instr_new_set(tc, z.loc));
			la = instr_new_label(NULL);
			lb = instr_new_label(NULL);
			block_emit(blk, la);
			block_emit(blk, instr_new_binop(td, ta, OP_GREATER, tb));
			block_emit(blk, instr_new_jumpif(lb, td));
			sa = scope_resolve_name(sco, st->range.ident);
			block_emit(blk, instr_new_set(sa->loc, ta));
			a = ir_visit_stmt(st->iter.body, blk, sco);
			block_append(blk, a);
			block_emit(blk, instr_new_binop(te, ta, OP_ADD, tc));
			block_emit(blk, instr_new_set(ta, te));
			block_emit(blk, instr_new_jump(la));
			block_emit(blk, lb);
			break;

		case ST_COMPOUND:
			for(i = 0; i < st->compound.stmts.len; i++) {
				a = ir_visit_stmt(vec_get(&st->compound.stmts, i, stmt_node), blk, sco);
				block_append(blk, a);
			}
			break;

		default:
			assert(0);
			break;
	}
	return blk;
}

ir_ev_res ir_visit_expr(expr_node *ex, block *pblk, scope *sco) {
	location *ta, *tb, *tc;
	block *blk = block_new(pblk);
	symbol *sa, *sb, *sc;
	switch(ex->kind) {
		case EX_LIT:
			switch(ex->lit.lit->kind) {
				case LIT_INT:
					break;
			}
			break;
	}
}
