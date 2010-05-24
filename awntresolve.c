#include <windows.h>

void * awosSelf() {
	return GetModuleHandle("npawplugin.dll"); // XXX
}

void * awosResolve(void * in, const char * name) {
	return GetProcAddress(in, name);
}
