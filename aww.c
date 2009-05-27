#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include "aw.h"
#include "awos.h"

#if !defined HANDLE_WM_MOUSEWHEEL // XXX mingw doesn't seem to define this one
#define HANDLE_WM_MOUSEWHEEL(hwnd,wParam,lParam,fn) ((fn)((hwnd),(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam),(int)(short)HIWORD(wParam),(UINT)(short)LOWORD(wParam)),0L)
#endif

#if !defined MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR 2
#endif

#define MAX_WINDOWS 256

struct _aw {
	awHeader hdr;
	HWND win;
	int lastmx, lastmy;
};

struct _ac {
	acHeader hdr;
	HGLRC ctx;
};

static aw * awFor(HWND win) {
	return (aw*)GetWindowLongPtrW(win, GWL_USERDATA);
}

static void wgot(HWND win, int type, int p1, int p2) {
	aw * w = awFor(win);
	if (w)
		got(w, type, p1, p2);
}

static void onMMove(HWND win, int x, int y, UINT flags ) {
	aw * w = awFor(win);
	if (x != w->lastmx || y != w->lastmy)
		got(w, AW_EVENT_MOTION, x, y);
	w->lastmx = x; w->lastmy = y;
}

static void onSize(HWND win, UINT state, int w, int h) {
	wgot(win, AW_EVENT_RESIZE, w, h);
}

static void onClose(HWND win){
	wgot(win, AW_EVENT_CLOSE, 0, 0);
}

static int vkMap(UINT vk) {
	int ret;
	switch (vk) {
	case VK_CONTROL:
		ret = AW_KEY_CONTROL; 
		break;
	case VK_SHIFT:
		ret = AW_KEY_SHIFT; 
		break;
	case VK_MENU:
		ret = AW_KEY_ALT;
		break;
	case VK_LWIN:
	case VK_RWIN:
		ret = AW_KEY_META;
		break;
	default:
		ret = MapVirtualKeyW(vk, MAPVK_VK_TO_CHAR);
		break;
	}
	report("mapped %d to %d", vk, ret);
	return ret;
}

static void onSysKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
	if (vk == VK_MENU)
		wgot(win, AW_EVENT_DOWN, vkMap(vk), 0);
}

static void onSysKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
	if (vk == VK_MENU)
		wgot(win, AW_EVENT_UP, vkMap(vk), 0);
}

static void onKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
	wgot(win, AW_EVENT_DOWN, vkMap(vk), 0);
}

static void onKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
	wgot(win, AW_EVENT_UP, vkMap(vk), 0);
}

static void onLD(HWND win, BOOL dbl, int x, int y, UINT flags) {
	wgot(win, AW_EVENT_DOWN, AW_KEY_MOUSELEFT, 0);
}

static void onMD(HWND win, BOOL dbl, int x, int y, UINT flags) {
	wgot(win, AW_EVENT_DOWN, AW_KEY_MOUSEMIDDLE, 0);
}

static void onRD(HWND win, BOOL dbl, int x, int y, UINT flags) {
	wgot(win, AW_EVENT_DOWN, AW_KEY_MOUSERIGHT, 0);
}

static void onLU(HWND win, int x, int y, UINT flags) {
	wgot(win, AW_EVENT_UP, AW_KEY_MOUSELEFT, 0);
}

static void onMU(HWND win, int x, int y, UINT flags) {
	wgot(win, AW_EVENT_UP, AW_KEY_MOUSEMIDDLE, 0);
}

static void onRU(HWND win, int x, int y, UINT flags) {
	wgot(win, AW_EVENT_UP, AW_KEY_MOUSERIGHT, 0);
}

static void onMW(HWND win, int x, int y, int z, UINT keys) {
	int which = z >= 0? AW_KEY_MOUSEWHEELUP : AW_KEY_MOUSEWHEELDOWN;
	wgot(win, AW_EVENT_DOWN, which, 0);
	wgot(win, AW_EVENT_UP, which, 0);
}

LONG WINAPI handle(HWND win, UINT msg, WPARAM w, LPARAM l)  {
	LONG r;
	int handled = 1;
	switch (msg) {
	case WM_NCCREATE: 
		SetWindowLongPtrW(
			win, GWL_USERDATA, 
			(LONG_PTR)((CREATESTRUCT*)l)->lpCreateParams);
		handled = 0;
		break;
	case WM_MOUSEMOVE: r = HANDLE_WM_MOUSEMOVE(win, w, l, onMMove); break;
	case WM_SIZE: r = HANDLE_WM_SIZE(win, w, l, onSize); break;
	case WM_CLOSE: r = HANDLE_WM_CLOSE(win, w, l, onClose); break;
	case WM_KEYDOWN: r = HANDLE_WM_KEYDOWN(win, w, l, onKeyDown); break;
	case WM_SYSKEYDOWN: r = HANDLE_WM_SYSKEYDOWN(win, w, l, onSysKeyDown); break;
	case WM_SYSKEYUP: r = HANDLE_WM_SYSKEYUP(win, w, l, onSysKeyUp); break;
	case WM_KEYUP: r = HANDLE_WM_KEYUP(win, w, l, onKeyUp); break;
	case WM_LBUTTONDOWN: r = HANDLE_WM_LBUTTONDOWN(win, w, l, onLD); break;
	case WM_RBUTTONDOWN: r = HANDLE_WM_RBUTTONDOWN(win, w, l, onRD); break;
	case WM_MBUTTONDOWN: r = HANDLE_WM_MBUTTONDOWN(win, w, l, onMD); break;
	case WM_LBUTTONUP: r = HANDLE_WM_LBUTTONUP(win, w, l, onLU); break;
	case WM_RBUTTONUP: r = HANDLE_WM_RBUTTONUP(win, w, l, onRU); break;
	case WM_MBUTTONUP: r = HANDLE_WM_MBUTTONUP(win, w, l, onMU); break;
	case WM_MOUSEWHEEL: r = HANDLE_WM_MOUSEWHEEL(win, w, l, onMW); break;
	default: handled = 0;
	}
	if (handled)
		r = 0;
	else
		r = DefWindowProcW(win, msg, w, l);
	return r;
}

static BOOL (APIENTRY *wglSwapIntervalEXT) (int interval) = 0;

int awosInit() {
	WNDCLASSW  wc;
	wglSwapIntervalEXT = (void*)wglGetProcAddress("wglSwapIntervalEXT");
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= handle;
	wc.hInstance		= GetModuleHandleW(NULL);
	wc.lpszClassName	= L"AW";
	return 0 != RegisterClassW(&wc);
}

int awosEnd() {
	return 1;
}

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

int awosSetTitle(aw * w, const char * t) {
	WCHAR wt[1024];
	MultiByteToWideChar(CP_UTF8, 0, t, strlen(t)+1, wt, sizeof(wt));
	return SetWindowTextW(w->win, wt);
	return 1;
}

static aw * openwin(int x, int y, int width, int height, 
		    DWORD style, DWORD exstyle) {
	aw * w = calloc(1, sizeof(*w));
	RECT r;
	HWND win;
	r.left = x; r.top = y;
	r.right = x + width; r.bottom = y + height;
	AdjustWindowRect(&r, style, FALSE);
	win = CreateWindowExW(exstyle, 
			      L"AW", L"AW", style,
			      r.left, r.top, 
			      r.right - r.left, r.bottom - r.top,
			      NULL, NULL, GetModuleHandle(NULL), w);
	if (win) {
		HDC dc = GetDC(win);
		setPF(dc);
		ReleaseDC(win, dc);
	}
	if (w)
		w->win = win;
	else
		free(w);
	return w;
}

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
	aw * w;
	DWORD style, exstyle;
#if 0
	if (fs) {
		RECT rect;
		GetWindowRect(GetDesktopWindow(), &rect);
		width = rect.right;
		height = rect.bottom;
	}
#endif
	if (bl) {
		style = WS_POPUP;
		exstyle = WS_EX_TOPMOST; 
	}
	else {
		style = 0
			| WS_OVERLAPPED
			| WS_CAPTION 
			| WS_SYSMENU 
			| WS_SIZEBOX
			| WS_MINIMIZEBOX
			| WS_MAXIMIZEBOX
			;
		exstyle = 0;
	}
//	if (fs)
//		style += WS_MAXIMIZE;
	w = openwin(x, y, width, height, style, exstyle);
	return w;
}

int awosClose(aw * w) {
	int ret = 1;
	if (w->win) ret &= 0 != DestroyWindow(w->win);
	free(w);
	return ret;
}

int awosSwapBuffers(aw * w) {
	int ret;
	HDC dc = GetDC(w->win);
	ret = SwapBuffers(dc);
	ReleaseDC(w->win, dc);
	return ret;
}

int awosMakeCurrent(aw * w, ac * c) {
	int ret;
	HDC dc = GetDC(w->win);
	ret = wglMakeCurrent(dc, c->ctx);
	ReleaseDC(w->win, dc);
	return ret;
}

int awosClearCurrent(aw * w) {
	return wglMakeCurrent(0, 0);
}

int awosShow(aw * w) {
	ShowWindow(w->win, w->hdr.fullscreen? SW_MAXIMIZE : SW_SHOWNORMAL);
	return 1;
}

int awosHide(aw * w) {
	ShowWindow(w->win, SW_HIDE);
	return 1;
}

static void dispatch(HWND win) {
	MSG msg;
	if (PeekMessageW(&msg, win, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void awosPollEvent(aw * w) {
	dispatch(w->win);
}

int awosSetSwapInterval(aw * w, int si) {
	return wglSwapIntervalEXT? wglSwapIntervalEXT(si) : 0;
}

ac * acosNew(ac * share) {
	ac * c = 0;
	HGLRC ctx;
	aw * dummy = awosOpen(0, 0, 0, 0, 0, 0);
	HDC dc = GetDC(dummy->win);
	setPF(dc);
	ctx = wglCreateContext(dc);
	ReleaseDC(dummy->win, dc);
	awosClose(dummy);
	if (ctx && share) 
		wglShareLists(share->ctx, ctx);
	if (ctx)
		c = calloc(1, sizeof(*c));
	if (c)
		c->ctx = ctx;
	return c;
}

int acosDel(ac * c) {
	int ret = 0 != wglDeleteContext(c->ctx);
	free(c);
	return ret;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
