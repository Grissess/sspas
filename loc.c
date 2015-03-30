#include <stdlib.h>
#include <assert.h>

#include "loc.h"

location *loc_new(void) {
	location *res = malloc(sizeof(location));
	assert(res);
	res->refcnt = 1;
	return res;
}

location *loc_copy(location *loc) {
	loc->refcnt++;
	return loc;
}

location *loc_new_mem(size_t addr) {
	location *res = loc_new();
	res->kind = LOC_MEM;
	res->mem.addr = addr;
	return res;
}

location *loc_new_ind(location *addr) {
	location *res = loc_new();
	res->kind = LOC_IND;
	res->ind.addr = loc_copy(addr);
	return res;
}

location *loc_new_off(location *addr, signed long amt) {
	location *res = loc_new();
	res->kind = LOC_OFF;
	res->off.addr = loc_copy(addr);
	res->off.amt = amt;
	return res;
}

location *loc_new_reg(char *regname) {
	location *res = loc_new();
	res->kind = LOC_REG;
	res->reg.reg = strdup(regname);
	return res;
}

void loc_delete(location *loc) {
	loc->refcnt--;
	if(loc->refcnt <= 0) loc_destroy(loc);
}

void loc_destroy(location *loc) {
	switch(loc->kind) {
		case LOC_MEM:
			break;

		case LOC_IND:
			loc_delete(loc->ind.addr);
			break;

		case LOC_OFF:
			loc_delete(loc->off.addr);
			loc_delete(loc->off.amt);
			break;

		case LOC_REG:
			free(loc->reg.reg);
			break;
	}
	free(loc);
}
