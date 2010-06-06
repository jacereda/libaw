#include <stdarg.h>
#include <stdio.h>

void report(const char * fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "AW ERROR: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
        fflush(stderr);
}

