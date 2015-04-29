#ifndef LOC_H
#define LOC_H

#include <stdlib.h>

typedef enum {
	LOC_MEM,
	LOC_IND,
	LOC_OFF,
	LOC_REG
} loc_k;

typedef struct _location location;

typedef struct _mem_location {
	size_t addr;
} mem_location;

typedef struct _ind_location {
	location *addr;
} ind_location;

typedef struct _off_location {
	location *addr;
	location *amt;
} off_location;

typedef struct _reg_location {
	char *reg;
} reg_location;

typedef struct _location {
	loc_k kind;
	size_t refcnt;
	union {
		mem_location mem;
		ind_location ind;
		off_location off;
		reg_location reg;
	};
} location;

location *loc_new(void);
location *loc_copy(location *loc);
location *loc_new_mem(size_t addr);
location *loc_new_ind(location *addr);
location *loc_new_off(location *addr, location *amt);
location *loc_new_reg(char *regname);
void loc_delete(location *loc);
void loc_destroy(location *loc);

#endif
