#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

void * awosSelf(const char * name) {
        char buf[256];
        snprintf(buf, sizeof buf - 1, "lib%s.so", name);
	return dlopen(buf, 0);
}

void * awosResolve(void * in, const char * name) {
	return dlsym(in, name);
}

const char * awosModPath() {
        static Dl_info info;
        dladdr(awosModPath, &info);
        return info.dli_fname;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
