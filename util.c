#include <string.h>
#include <stdarg.h>

#include "util.h"

int string_equal(const char *sa, const char *sb) {
	return !strcmp(sa, sb);
}

void wrlev(FILE *f, int lev, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	size_t i;
	for(i = 0; i < lev; i++) {
		fputc(' ', f);
		fputc(' ', f);
	}
	vfprintf(f, fmt, va);
	fputc('\n', f);
	va_end(va);
}
