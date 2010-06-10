#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

void * awosSelf(const char * name) {
        char buf[256];
        snprintf(buf, sizeof buf - 1, "lib%s.so", name);
	return dlopen(buf, 0);
}

void * awosResolve(void * in, const char * name) {
	return dlsym(in, name);
}

const char * awosModName() {
        Dl_info info;
        static char buf[256];
        dladdr(awosModName, &info);
        snprintf(buf, sizeof buf - 1, "%s", strrchr(info.dli_fname, '/')+1);
        *strrchr(buf, '.') = 0;
        report("%s", buf+3);
        return buf+3;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
