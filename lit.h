#ifndef LIT_H
#define LIT_H

#include <stdlib.h>

#include "type.h"
#include "vector.h"

typedef enum {
	LIT_INT,
	LIT_REAL,
	LIT_CHAR,
	LIT_ARRAY,
} lit_k;

typedef struct _literal {
	lit_k kind;
	size_t refcnt;
	type *type;
	union {
		long ival;
		double fval;
		char cval;
		vector items; /* of literal * */
	};
} literal;

literal *lit_new(void);
literal *lit_copy(literal *lit);
literal *lit_new_int(long ival);
literal *lit_new_real(double fval);
literal *lit_new_char(char cval);
literal *lit_new_array(vector *init,type *fallback);
void lit_delete(literal *lit);
void lit_destroy(literal *lit);

#endif
