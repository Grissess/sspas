#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "loc.h"
#include "vector.h"

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

location *loc_new_mem(ssize_t addr) {
	location *res = loc_new();
	res->kind = LOC_MEM;
	res->mem.addr = addr;
	return res;
}

location *loc_new_temp(void *data) {
	location *res = loc_new();
	res->kind = LOC_TEMP;
	res->temp.data = data;
	return res;
}

location *loc_new_ind(location *addr) {
	location *res = loc_new();
	res->kind = LOC_IND;
	res->ind.addr = loc_copy(addr);
	return res;
}

location *loc_new_off(location *addr, location *amt) {
	location *res = loc_new();
	res->kind = LOC_OFF;
	res->off.addr = loc_copy(addr);
	res->off.amt = amt;
	return res;
}

location *loc_new_off_vec(location *addr, vector *amts) {
	size_t i;
	location *res, *prev;
	if(!amts->len) {
		return loc_copy(addr);
	}
	prev = addr;
	for(i = 0; i < amts->len; i++) {
		res = loc_new_off(prev, vec_get(amts, i, location));
		prev = res;
	}
	return res;
}

location *loc_new_stride(location *loc, location *stride) {
	location *res = loc_new();
	res->kind = LOC_STRIDE;
	res->stride.loc = loc_copy(loc);
	res->stride.stride = loc_copy(stride);
	return res;
}

location *loc_new_reg(reg_k kind) {
	location *res = loc_new();
	res->kind = LOC_REG;
	res->reg.kind = kind;
	return res;
}

location *loc_new_sym(char *name) {
	location *res = loc_new();
	res->kind = LOC_SYM;
	res->sym.name = strdup(name);
	return res;
}

char *SYNAME_GDISP = "__GLOBAL_DISPLAY_TABLE";

location *loc_new_size(type *ty) {
	location *res = loc_new();
	res->kind = LOC_SIZE;
	if(ty) res->size.type = type_copy(ty);
	else res->size.type = NULL;
	return res;
}

#define LREPR_SZ 1024

static const char *REG_NAMES[] = {
	"FP",
	"SP",
};

char *loc_repr(location *loc) {
	char lbuffer[LREPR_SZ] = {0};
	char *lrepr = malloc(sizeof(char) * LREPR_SZ);
	int chars;
	if(!loc) {
		snprintf(lrepr, LREPR_SZ, "NULL");
		return lrepr;
	}
	switch(loc->kind) {
		case LOC_TEMP:
			chars = snprintf(lbuffer, LREPR_SZ, "<temp>");
			break;

		case LOC_MEM:
			chars = snprintf(lbuffer, LREPR_SZ, "%ld", loc->mem.addr);
			break;

		case LOC_IND:
            chars = snprintf(lbuffer, LREPR_SZ, "*(%s)", loc_repr(loc->ind.addr));
			break;

		case LOC_OFF:
			chars = snprintf(lbuffer, LREPR_SZ, "(%s)+(%s)", loc_repr(loc->off.addr), loc_repr(loc->off.amt));
			break;

		case LOC_STRIDE:
			chars = snprintf(lbuffer, LREPR_SZ, "(%s)*(%s)", loc_repr(loc->stride.loc), loc_repr(loc->stride.stride));
			break;

		case LOC_REG:
			chars = snprintf(lbuffer, LREPR_SZ, "%s", REG_NAMES[loc->reg.kind]);
			break;

		case LOC_SYM:
			chars = snprintf(lbuffer, LREPR_SZ, "&%s", loc->sym.name);
			break;

		case LOC_SIZE:
			chars = snprintf(lbuffer, LREPR_SZ, "sizeof(%s)", type_repr(loc->size.type));
			break;

		default:
			assert(0);
			break;
	}
	memcpy(lrepr, lbuffer, chars + 1);
	return lrepr;
}

void loc_delete(location *loc) {
	loc->refcnt--;
	if(loc->refcnt <= 0) {
		loc_destroy(loc);
	}
}

void loc_destroy(location *loc) {
	switch(loc->kind) {
		case LOC_MEM: case LOC_TEMP:
			break;

		case LOC_IND:
			loc_delete(loc->ind.addr);
			break;

		case LOC_OFF:
			loc_delete(loc->off.addr);
			loc_delete(loc->off.amt);
			break;

		case LOC_REG:
			break;

		case LOC_SYM:
			free(loc->sym.name);
			break;

		case LOC_SIZE:
			type_delete(loc->size.type);
			break;

		default:
			assert(0);
			break;
	}
	free(loc);
}
