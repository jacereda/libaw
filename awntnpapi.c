#include "awnpapios.h"
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

struct _ins {
	insHeader h;
	WNDPROC oldproc;
};

static void setPF(HDC dc) {
        PIXELFORMATDESCRIPTOR pfd = { 
                sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
                1,                     // version number 
                PFD_DRAW_TO_WINDOW |   // support window 
                PFD_SUPPORT_OPENGL |   // support OpenGL 
                PFD_DOUBLEBUFFER,      // double buffered 
                PFD_TYPE_RGBA,         // RGBA type 
                24,                    // 24-bit color depth 
                0, 0, 0, 0, 0, 0,      // color bits ignored 
                0,                     // no alpha buffer 
                0,                     // shift bit ignored 
                0,                     // no accumulation buffer 
                0, 0, 0, 0,            // accum bits ignored 
                32,                    // 32-bit z-buffer 
                0,                     // no stencil buffer 
                0,                     // no auxiliary buffer 
                PFD_MAIN_PLANE,        // main layer 
                0,                     // reserved 
                0, 0, 0                // layer masks ignored 
        }; 
        SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd); 
}

ins * awosNew(NPNetscapeFuncs * browser, NPP i) {
	return calloc(1, sizeof(ins));
}

LONG WINAPI plghandle(HWND win, UINT msg, WPARAM w, LPARAM l) {
	extern LONG WINAPI handle(HWND win, UINT msg, WPARAM w, LPARAM l);
	LONG r;
	switch (msg) {
	case WM_TIMER: 
		PostMessage(win, WM_PAINT, 0, 0);
		break;
	case WM_PAINT: 
	{
		insHeader * hdr = (insHeader*)GetWindowLongPtrW(win, GWL_USERDATA);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(win, &ps);
		coSwitchTo(hdr->coaw);
		EndPaint(win, &ps);
		r = 0;
	}
	break;
	default:
		r = handle(win, msg, w, l);
	}
	return r;
}

void awosSetWindow(ins * o, void * vwin) {
	HANDLE win = (HANDLE)vwin;
	HDC dc;
	if (!o->oldproc) {
		o->h.w.handle = vwin;
		o->oldproc = SubclassWindow(win, plghandle);
		SetWindowLongPtrW(win, GWL_USERDATA, (LONG_PTR)o);
		dc = GetDC(win);
		setPF(dc);
		o->h.c.handle = wglCreateContext(dc);
		wglMakeCurrent(dc, o->h.c.handle);
		ReleaseDC(win, dc);
		SetTimer(win, 1, 20, 0);
	}
}

void awosDel(ins * o) {
	if (o->oldproc)
		SubclassWindow(o->h.w.handle, o->oldproc);
	free(o);
}


static BOOL (APIENTRY *wglSwapInterval) (int interval) = 0;

int awosMakeCurrentI(ins * o) {
	HANDLE win = (HANDLE)o->h.w.handle;
	HDC dc = GetDC(win);
	int ret;
	ret = wglMakeCurrent(dc, (HGLRC)o->h.c.handle);
        if (!wglSwapInterval)
                wglSwapInterval = (void*)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapInterval)
		wglSwapInterval(1);
	ReleaseDC(win, dc);
	return ret;
}

int awosClearCurrentI(ins * o) {
        return wglMakeCurrent(0, 0);
}

void awosUpdate(ins * o) {
        HDC dc = GetDC(o->h.w.handle);
        SwapBuffers(dc);
        ReleaseDC(o->h.w.handle, dc);
}

NPError awosEvent(ins * o, void * ev) {
	NPEvent * e = (NPEvent *)ev;
	report("osevent");
	return NPERR_NO_ERROR;
}

const char * awosResourcesPath(ins * o) {
	return ".";
}

NPError awosGetValue(NPP i, NPPVariable var, void * v) {
	ins * o = (ins*)i->pdata;
	NPError ret;
	report("os getvalue"); 
	switch(var) {
	default: 
		report("os getval default"); 
		ret = NPERR_GENERIC_ERROR; 
		break;
	}
	return ret;
} 
