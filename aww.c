#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include "aw.h"
#include "awos.h"

#define MAX_EVENTS 256
#define MAX_WINDOWS 256


struct _aw {
	awHeader hdr;
	HWND win;
	HGLRC ctx;
	HDC pushdc;
	HGLRC pushctx;
	awEvent ev[MAX_EVENTS];
	unsigned head, tail;
	int lastmx, lastmy;
};

static aw * g_w[MAX_WINDOWS];

unsigned indexFor(HWND win) {
	unsigned i = MAX_WINDOWS;
	unsigned ret = ~0;
	while (i--) {
		aw * w = g_w[i];
		if (w && w->win == win)
			ret = i;
	}
	return ret;
}

unsigned emptySlot() {
	unsigned i = MAX_WINDOWS;
	unsigned ret = ~0;
	while (i--)
		if (!g_w[i])
			ret = i;
	return ret;
}

aw * awFor(HWND win) {
	return g_w[indexFor(win)];
}

static void got(HWND win, int type, int p1, int p2) {
	aw * w = awFor(win);
	if (w) {
		awEvent * e = w->ev + w->head;
		w->head++;
		w->head %= MAX_EVENTS;
		e->type = type;
		e->u.p[0] = p1;
		e->u.p[1] = p2;
	}
}

static void onMMove(HWND win, int x, int y, UINT flags ) {
	aw * w = awFor(win);
	if (x != w->lastmx || y != w->lastmy)
		got(win, AW_EVENT_MOTION, x, y);
	w->lastmx = x; w->lastmy = y;
}

static void onMove(HWND win, int x, int y) {
	got(win, AW_EVENT_MOVE, x, y);
}

static void onSize(HWND win, UINT state, int w, int h) {
	got(win, AW_EVENT_RESIZE, w, h);
}

static void onClose(HWND win){
	got(win, AW_EVENT_CLOSE, 0, 0);
}

static void onKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
	got(win, AW_EVENT_DOWN, vk, 0);
}

static void onKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
	got(win, AW_EVENT_UP, vk, 0);
}

static void onLD(HWND win, BOOL dbl, int x, int y, UINT flags) {
	got(win, AW_EVENT_DOWN, AW_KEY_MOUSELEFT, 0);
}

static void onMD(HWND win, BOOL dbl, int x, int y, UINT flags) {
	got(win, AW_EVENT_DOWN, AW_KEY_MOUSEMIDDLE, 0);
}

static void onRD(HWND win, BOOL dbl, int x, int y, UINT flags) {
	got(win, AW_EVENT_DOWN, AW_KEY_MOUSERIGHT, 0);
}

static void onLU(HWND win, int x, int y, UINT flags) {
	got(win, AW_EVENT_UP, AW_KEY_MOUSELEFT, 0);
}

static void onMU(HWND win, int x, int y, UINT flags) {
	got(win, AW_EVENT_UP, AW_KEY_MOUSEMIDDLE, 0);
}

static void onRU(HWND win, int x, int y, UINT flags) {
	got(win, AW_EVENT_UP, AW_KEY_MOUSERIGHT, 0);
}

static void onMW(HWND win, int x, int y, int z, UINT keys) {
	int which = z >= 0? AW_KEY_MOUSEWHEELUP : AW_KEY_MOUSEWHEELDOWN;
	got(win, AW_EVENT_DOWN, which, 0);
	got(win, AW_EVENT_UP, which, 0);
}

LONG WINAPI handle(HWND win, UINT msg, WPARAM w, LPARAM l)  {
	LONG r;
	int handled = 1;
	switch (msg) {
	case WM_MOUSEMOVE: r = HANDLE_WM_MOUSEMOVE(win, w, l, onMMove); break;
	case WM_MOVE: r = HANDLE_WM_MOVE(win, w, l, onMove); break;
	case WM_SIZE: r = HANDLE_WM_SIZE(win, w, l, onSize); break;
	case WM_CLOSE: r = HANDLE_WM_CLOSE(win, w, l, onClose); break;
	case WM_KEYDOWN: r = HANDLE_WM_KEYDOWN(win, w, l, onKeyDown); break;
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
		r = DefWindowProc(win, msg, w, l);
	return r;
}

int awosInit() {
	WNDCLASS  wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= handle;
	wc.hInstance		= GetModuleHandle(NULL);
	wc.lpszClassName	= L"AW";
	return 0 != RegisterClass(&wc);
}

void awosEnd() {
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

aw * awosOpen(const char * t, int x, int y, int width, int height, void * ct) {
	aw * ret = NULL;
	aw * w = calloc(1, sizeof(*ret));
	HDC dc;
	RECT r;
	DWORD style = 0
		| WS_OVERLAPPED
		| WS_CAPTION 
		| WS_SYSMENU 
		| WS_SIZEBOX
		| WS_MINIMIZEBOX
		| WS_CLIPSIBLINGS
		| WS_CLIPCHILDREN
		;
	r.left = x; r.top = y;
	r.right = x + width; r.bottom = y + height;
	AdjustWindowRect(&r, style, FALSE);
	w->win = CreateWindow(L"AW", L"AW", style,
			      r.left, r.top, 
			      r.right - r.left, r.bottom - r.top,
			      NULL, NULL, GetModuleHandle(NULL), NULL);
	if (w->win) {
		dc = GetDC(w->win);
		setPF(dc);
		w->ctx = wglCreateContext(dc);
		if (ct) 
			wglShareLists(ct, w->ctx);
		ReleaseDC(w->win, dc);
	}
	if (w->win && w->ctx) {
		ret = w;
		g_w[emptySlot()] = w;
	}
	else
		awosClose(w);
	return ret;
}

int awosClose(aw * w) {
	int ret = 1;
	wglMakeCurrent(0, 0);
	if (w->win && w->ctx) g_w[indexFor(w->win)] = 0;
	if (w->win) ret &= 0 != DestroyWindow(w->win);
	if (w->ctx) ret &= 0 != wglDeleteContext(w->ctx);
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

int awosPushCurrent(aw * w) {
	int ret;
	HDC dc = GetDC(w->win);
	w->pushdc = wglGetCurrentDC();
	w->pushctx = wglGetCurrentContext();
	ret = wglMakeCurrent(dc, w->ctx);
	ReleaseDC(w->win, dc);
	return ret;
}

int awosPopCurrent(aw * w) {
	return wglMakeCurrent(w->pushdc, w->pushctx);
}


int awosShow(aw * w) {
	ShowWindow(w->win, SW_SHOWNORMAL);
	return 1;
}

int awosHide(aw * w) {
	ShowWindow(w->win, SW_HIDE);
	return 1;
}

int awosSetTitle(aw * w, const char * t) {
	WCHAR wt[1024];
	MultiByteToWideChar(CP_UTF8, 0, t, strlen(t)+1, wt, sizeof(wt));
	return SetWindowText(w->win, wt);
}

static void toGlobal(aw * w, int x, int y, int * gx, int * gy) {
	POINT p;
	p.x = x; p.y = y;
	MapWindowPoints(w->win, GetDesktopWindow(), &p, 1);
	*gx = p.x;
	*gy = p.y;
}

static void getSize(aw * w, int * width, int * height) {
	RECT r;
	GetClientRect(w->win, &r);	
	*width = r.right - r.left; *height = r.bottom - r.top;
}

static void getPos(aw * w, int * x, int * y) {
	POINT p;
	p.x = 0; p.y = 0;
	ClientToScreen(w->win, &p);
	*x = p.x; *y = p.y;
}

static int setGeom(aw * w, int x, int y, int width, int height) {
	RECT r;
	r.left = x; r.top = y;
	r.right = x + width; r.bottom = y + height;
	AdjustWindowRect(&r, GetWindowLong(w->win, GWL_STYLE), FALSE);
	return MoveWindow(w->win, r.left, r.top, 
			  r.right - r.left, r.bottom - r.top, 1);
}

int awosResize(aw * w, int width, int height) {
	int x, y;
	getPos(w, &x, &y);
	return setGeom(w, x, y, width, height);
}

static void dispatch(HWND win) {
	MSG msg;
	PeekMessage(&msg, win, 0, 0, PM_REMOVE);
	TranslateMessage(&msg);
	DispatchMessage(&msg);
}

void awosNextEvent(aw * w) {
	dispatch(w->win);
	if (w->head != w->tail) {
		w->hdr.next = w->ev[w->tail];
		w->tail++;
		w->tail %= MAX_EVENTS;
	}
}

int awosSetSwapInterval(int si) {
	static BOOL (APIENTRY *wglSwapIntervalEXT) (int interval) = 0;
	if (!wglSwapIntervalEXT)
		wglSwapIntervalEXT = (void*)wglGetProcAddress("wglSwapIntervalEXT");
	return wglSwapIntervalEXT(si);
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   End: **
*/
