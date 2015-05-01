#ifndef CG_H
#define CG_H

#include "vector.h"
#include "ast.h"
#include "sem.h"
#include "loc.h"

typedef struct _block block;

typedef enum {
	IN_SET,
	IN_LADDR,
	IN_BINOP,
	IN_UNOP,
	IN_PUSH,
	IN_POP,
	IN_CALL,
	IN_RETURN,
	IN_JUMP,
	IN_JUMPIF,
	IN_LABEL,
} instr_k;

typedef struct _set_instr {
	location *loc;
	location *value;
} set_instr;

typedef struct _laddr_instr {
	location *loc;
	location *value;
} laddr_instr;

typedef struct _binop_instr {
	location *loc;
	binop_k kind;
	location *left;
	location *right;
} binop_instr;

typedef struct _unop_instr {
	location *loc;
	unop_k kind;
	location *value;
} unop_instr;

typedef struct _push_instr {
	location *value;
} push_instr;

typedef struct _pop_instr {
	location *loc;
} pop_instr;

typedef struct _call_instr {
	block *label;
} call_instr;

typedef struct _return_instr {
	location *value;
} return_instr;

typedef struct _jump_instr {
	block *label;
} jump_instr;

typedef struct _jumpif_instr {
	block *label;
	location *test;
} jumpif_instr;

typedef struct _label_instr {
	char *name;
} label_instr;

typedef struct _instr {
	instr_k kind;
	union {
		set_instr set;
		laddr_instr laddr;
		binop_instr binop;
		unop_instr unop;
		push_instr push;
		pop_instr pop;
		call_instr call;
		return_instr return_;
		jump_instr jump;
		jumpif_instr jumpif;
		label_instr label;
	};
} instr;

instr *instr_new(void);
instr *instr_new_set(location *loc,location *value);
instr *instr_new_laddr(location *loc,location *value);
instr *instr_new_binop(location *loc,location *left,binop_k kind,location *right);
instr *instr_new_unop(location *loc,unop_k kind,location *value);
instr *instr_new_push(location *value);
instr *instr_new_pop(location *loc);
instr *instr_new_call(block *label);
instr *instr_new_return(location *value);
instr *instr_new_jump(block *label);
instr *instr_new_jumpif(block *label,location *test);
instr *instr_new_label(char *);
instr *instr_copy(instr *ins);
void instr_print(FILE *, int, instr *);
void instr_delete(instr *ins);
void instr_destroy(instr *ins);

typedef enum {
	BLK_ROOT,
	BLK_PROG,
	BLK_LABEL,
} block_k;

typedef struct _block {
	block_k kind;
	struct _block *parent;
	vector children; /* of block * */
	vector instrs; /* of instr * */
	union {
		program *prog;
		stmt_node *stmt;
	};
} block;

block *block_new(block *parent);
block *block_new_program(block *parent,program *prog);
block *block_new_stmt(block *parent,stmt_node *stmt);
block *block_copy(block *blk);
void block_print(FILE *, int, block *);
char *block_repr(block *);
void block_delete(block *blk);
void block_destroy(block *blk);

#endif
