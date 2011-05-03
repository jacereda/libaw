#include <stdarg.h>
#include <stdio.h>
#include "aw.h"
#include "awos.h"

void report(const char *fmt, ...) {
#if defined _WIN32 || defined __ANDROID__
	FILE *out = fopen("aw.log", "a");
#else
	FILE *out = fopen("/tmp/aw.log", "a");
#endif
	va_list ap;
	va_start(ap, fmt);
	if(out) {
		vfprintf(out, fmt, ap);
		fputs("\n", out);
		fclose(out);
	}
	va_end(ap);
}


