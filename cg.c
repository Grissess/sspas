#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cg.h"
#include "sem.h"
#include "ast.h"
#include "util.h"

block *block_new(block *parent) {
	block *res = malloc(sizeof(block));
	if(parent) {
		res->parent = block_copy(parent);
		vec_insert(&parent->children, parent->children.len, block_copy(res));
	} else {
		res->parent = NULL;
	}
	res->kind = BLK_ROOT;
	vec_init(&res->children);
	vec_init(&res->instrs);
	return res;
}

block *block_new_program(block *parent, program *prog) {
	block *res = block_new(parent);
	res->kind = BLK_PROG;
	res->prog = program_copy(prog);
	return res;
}

block *block_new_stmt(block *parent, stmt_node *stmt) {
	block *res = block_new(parent);
	res->kind = BLK_LABEL;
	res->stmt = st_copy(stmt);
	return res;
}

block *block_copy(block *blk) {
	return blk;
}

void block_emit(block *blk, instr *ins) {
	vec_insert(&blk->instrs, blk->instrs.len, instr_copy(ins));
}

void block_append(block *blk, block *subblk) {
	size_t i;
	for(i = 0; i < subblk->instrs.len; i++) {
		block_emit(blk, vec_get(&subblk->instrs, i, instr));
	}
}

void block_delete(block *blk) {
	block_destroy(blk);
}

void block_destroy(block *blk) {
	switch(blk->kind) {
		case BLK_ROOT:
			break;

		case BLK_PROG:
			program_delete(blk->prog);
			break;

		case BLK_LABEL:
			st_delete(blk->stmt);
			break;

		default:
			assert(0);
			break;
	}
	free(blk);
}

void block_print(FILE *out, int lev, block *blk) {
	size_t i;
	if(!blk) {
		wrlev(out, lev, "-NULL-");
		return;
	}
	switch(blk->kind) {
		case BLK_ROOT:
			wrlev(out, lev, "-ROOT (%p)-", blk);
			break;

		case BLK_PROG:
			wrlev(out, lev, "-PROG %s (%p)-", blk->prog->node->ident, blk);
			break;

		case BLK_LABEL:
			wrlev(out, lev, "-LABEL (%p)-", blk);
			break;

		default:
			wrlev(out, lev, "-!!!UNKNOWN BLOCK %d (%p)!!!-", blk->kind, blk);
			break;
	}
	for(i = 0; i < blk->instrs.len; i++) {
		instr_print(out, lev + 1, vec_get(&blk->instrs, i, instr));
	}
	for(i = 0; i < blk->children.len; i++) {
		block_print(out, lev + 1, vec_get(&blk->children, i, block));
	}
}

char *block_repr(block *blk) {
	char *res = malloc(sizeof(char) * 32);
	snprintf(res, 32, "(block %p)", blk);
	return res;
}

instr *instr_new(void) {
	instr *res = malloc(sizeof(instr));
	return res;
}

instr *instr_new_set(location *loc, location *value) {
	instr *res = instr_new();
	res->kind = IN_SET;
	res->set.loc = loc_copy(loc);
	res->set.value = loc_copy(value);
	return res;
}

instr *instr_new_laddr(location *loc, location *value) {
	instr *res = instr_new();
	res->kind = IN_LADDR;
	res->laddr.loc = loc_copy(loc);
	res->laddr.value = loc_copy(value);
	return res;
}

instr *instr_new_binop(location *loc, location *left, binop_k kind, location *right) {
	instr *res = instr_new();
	res->kind = IN_BINOP;
	res->binop.left = loc_copy(left);
	res->binop.kind = kind;
	res->binop.right = loc_copy(right);
	res->binop.loc = loc_copy(loc);
	return res;
}

instr *instr_new_unop(location *loc, unop_k kind, location *value) {
	instr *res = instr_new();
	res->kind = IN_UNOP;
	res->unop.kind = kind;
	res->unop.value = loc_copy(value);
	res->unop.loc = loc_copy(loc);
	return res;
}

instr *instr_new_push(location *value) {
	instr *res = instr_new();
	res->kind = IN_PUSH;
	res->push.value = loc_copy(value);
	return res;
}

instr *instr_new_pop(location *loc) {
	instr *res = instr_new();
	res->kind = IN_POP;
	res->pop.loc = loc_copy(loc);
	return res;
}

instr *instr_new_call(block *label) {
	instr *res = instr_new();
	res->kind = IN_CALL;
	res->call.label = block_copy(label);
	return res;
}

instr *instr_new_return(location *value) {
	instr *res = instr_new();
	res->kind = IN_RETURN;
	res->return_.value = loc_copy(value);
	return res;
}

instr *instr_new_jump(block *label) {
	instr *res = instr_new();
	res->kind = IN_JUMP;
	res->jump.label = block_copy(label);
	return res;
}

instr *instr_new_jumpif(block *label, location *test) {
	instr *res = instr_new();
	res->kind = IN_JUMPIF;
	res->jumpif.label = block_copy(label);
	res->jumpif.test = loc_copy(test);
	return res;
}

vector all_labels;
unsigned long next_num_label = 0;

instr *instr_new_label(char *label) {
	instr *res;
	size_t i;
	if(label) {
		for(i = 0; i < all_labels.len; i++) {
			if(string_equal(label, vec_get(&all_labels, i, instr)->label.name)) {
				return instr_copy(vec_get(&all_labels, i, instr));
			}
		}
	}
	res = instr_new();
	res->kind = IN_LABEL;
	if(!label) {
		res->label.name = malloc(sizeof(char) * 32);
		snprintf(res->label.name, 32, "__L%ld__", next_num_label++);
	} else {
		res->label.name = strdup(label);
	}
	vec_insert(&all_labels, all_labels.len, res);
	return res;
}

instr *instr_copy(instr *ins) {
	return ins;
}

void instr_delete(instr *ins) {
	instr_destroy(ins);
}

void instr_destroy(instr *ins) {
	switch(ins->kind) {
		case IN_SET:
			loc_delete(ins->set.loc);
			loc_delete(ins->set.value);
			break;

		case IN_LADDR:
			loc_delete(ins->laddr.loc);
			loc_delete(ins->laddr.value);
			break;

		case IN_BINOP:
			loc_delete(ins->binop.left);
			loc_delete(ins->binop.right);
			loc_delete(ins->binop.loc);
			break;

		case IN_UNOP:
			loc_delete(ins->unop.value);
			loc_delete(ins->unop.loc);
			break;

		case IN_PUSH:
			loc_delete(ins->push.value);
			break;

		case IN_POP:
			loc_delete(ins->pop.loc);
			break;

		case IN_CALL:
			block_delete(ins->call.label);
			break;

		case IN_RETURN:
			loc_delete(ins->return_.value);
			break;

		case IN_JUMP:
			block_delete(ins->jump.label);
			break;

		case IN_JUMPIF:
			block_delete(ins->jumpif.label);
			loc_delete(ins->jumpif.test);
			break;

		default:
			assert(0);
			break;
	}
	free(ins);
}

void instr_print(FILE *out, int lev, instr *ins) {
	if(!ins) {
		wrlev(out, lev, ".NULL");
		return;
	}
	switch(ins->kind) {
		case IN_SET:
			wrlev(out, lev, ".SET %s <- %s", loc_repr(ins->set.loc), loc_repr(ins->set.value));
			break;

		case IN_LADDR:
			wrlev(out, lev, ".LADDR %s =& %s", loc_repr(ins->laddr.loc), loc_repr(ins->laddr.loc));
			break;

		case IN_BINOP:
			wrlev(out, lev, ".BINOP %s = %s %d %s", loc_repr(ins->binop.loc), loc_repr(ins->binop.left), ins->binop.kind, loc_repr(ins->binop.right));
			break;

		case IN_UNOP:
			wrlev(out, lev, ".UNOP %s = %d %s", loc_repr(ins->unop.loc), ins->unop.kind, loc_repr(ins->unop.value));
			break;

		case IN_PUSH:
			wrlev(out, lev, ".PUSH %s", loc_repr(ins->push.value));
			break;

		case IN_POP:
			wrlev(out, lev, ".POP %s", loc_repr(ins->pop.loc));
			break;

		case IN_CALL:
			wrlev(out, lev, ".CALL %s", block_repr(ins->call.label));
			break;

		case IN_RETURN:
			wrlev(out, lev, ".RETURN %s", loc_repr(ins->return_.value));
			break;

		case IN_JUMP:
			wrlev(out, lev, ".JUMP %s", block_repr(ins->jump.label));
			break;

		case IN_JUMPIF:
			wrlev(out, lev, ".JUMP %s IF %s", block_repr(ins->jumpif.label), loc_repr(ins->jumpif.test));
			break;

		default:
			wrlev(out, lev, ".!!!UNKNOWN INSTR %d!!!", ins->kind);
			break;
	}
}
