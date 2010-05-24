#include <dlfcn.h>

void * awosSelf() {
	return dlopen(0, 0);
}

void * awosResolve(void * in, const char * name) {
	return dlsym(in, name);
}

