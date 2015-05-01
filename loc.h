#ifndef LOC_H
#define LOC_H

#include <stdlib.h>

#include "type.h"
#include "vector.h"

typedef enum {
	LOC_TEMP,
	LOC_MEM,
	LOC_IND,
	LOC_OFF,
	LOC_STRIDE,
	LOC_REG,
	LOC_SYM,
	LOC_SIZE,
} loc_k;

typedef struct _location location;

typedef struct _mem_location {
	ssize_t addr;
} mem_location;

typedef struct _temp_location {
	void *data;
} temp_location;

typedef struct _ind_location {
	location *addr;
} ind_location;

typedef struct _off_location {
	location *addr;
	location *amt;
} off_location;

typedef struct _stride_location {
	location *loc;
	location *stride;
} stride_location;

typedef enum {
	REG_FP,
	REG_SP,
} reg_k;

typedef struct _reg_location {
	reg_k kind;
} reg_location;

typedef struct _sym_location {
	char *name;
} sym_location;

extern char *SYNAME_GDISP;

typedef struct _size_location {
	type *type;
} size_location;

typedef struct _location {
	loc_k kind;
	size_t refcnt;
	union {
		mem_location mem;
		temp_location temp;
		ind_location ind;
		off_location off;
		stride_location stride;
		reg_location reg;
		sym_location sym;
		size_location size;
	};
} location;

location *loc_new(void);
location *loc_new_temp(void *);
location *loc_copy(location *loc);
location *loc_new_mem(ssize_t addr);
location *loc_new_ind(location *addr);
location *loc_new_off(location *addr, location *amt);
location *loc_new_off_vec(location *addr, vector *amts);
location *loc_new_stride(location *loc, location *stride);
location *loc_new_reg(reg_k);
location *loc_new_sym(char *);
location *loc_new_size(type *);
char *loc_repr(location *);
void loc_delete(location *loc);
void loc_destroy(location *loc);

#endif
