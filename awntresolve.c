#include <windows.h>

void * awosSelf() {
	HMODULE ret;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
			  (LPCSTR)awosSelf,  &ret);
	return ret;
}

void * awosResolve(void * in, const char * name) {
	return GetProcAddress(in, name);
}

const char * awosModPath() {
	static char buf[256];
	GetModuleFileName(awosSelf(), buf, sizeof(buf));
	return buf;
}
