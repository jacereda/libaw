#include <stdarg.h>
#include <stdio.h>
#if defined AWPLUGIN
extern void report(const char * fmt, ...);
#define Log report
#else
static void Log(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	fflush(stdout);
	va_end(ap);
}
#endif

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
