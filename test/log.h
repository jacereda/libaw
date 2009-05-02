#include <stdarg.h>
#include <stdio.h>
static void Log(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	fflush(stdout);
	va_end(ap);
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
