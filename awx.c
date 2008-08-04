#include <stdlib.h>
#include <GL/glx.h>

#include "aw.h"
#include "awos.h"

#if 0 // defined(__APPLE__)
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/OpenGL.h>
int setSwapInterval(long interval) {
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);
	return 1;
}
#else
static int (*glXSwapIntervalSGI)(unsigned interval) = 0;
int setSwapInterval(int interval) {
	return 0 == glXSwapIntervalSGI(interval);
}
#endif



#define EVMASK 	KeyPressMask | ButtonPressMask | ButtonReleaseMask |\
                PointerMotionMask | StructureNotifyMask
static Display * g_dpy;
static Atom g_del;
static int g_screen;

#define MAX_EVENTS 8

struct _aw {
	awHeader hdr;
	Window win;
	GLXContext ctx;
	Window pushwin;
	GLXContext pushctx;
	XVisualInfo * vinfo;
	awEvent ev[MAX_EVENTS];
	unsigned head, tail;
	int x, y, w, h;
	int lastx, lasty, lastw, lasth;
};

static int sync() {
	return XSync(g_dpy, False);
}

static void got(aw  * w, int type, int p1, int p2) {
	awEvent * e = w->ev + w->head;
	w->head++;
	w->head %= MAX_EVENTS;
	e->type = type;
	e->u.p[0] = p1;
	e->u.p[1] = p2;
}

static XVisualInfo* chooseVisual() {
	int att[64];
	int * p = att;
	*p++ = GLX_RGBA;
	*p++ = GLX_DOUBLEBUFFER;
	*p++ = GLX_RED_SIZE; *p++ = 1;
	*p++ = GLX_GREEN_SIZE; *p++ = 1;
	*p++ = GLX_BLUE_SIZE; *p++ = 1;
	*p++ = GLX_DEPTH_SIZE; *p++ = 1;
	*p++ = None;
	return glXChooseVisual(g_dpy, g_screen, att);
}

static void fillWA(unsigned long * swam, XSetWindowAttributes * swa) {
	*swam = 0;
	swa->event_mask = EVMASK; *swam |= CWEventMask;
	swa->border_pixel = 0; *swam = CWBorderPixel;
	swa->colormap = XDefaultColormap(g_dpy, g_screen); *swam = CWColormap;
}

static int reconfigure(aw * w) {
	int ret = 0;
	XWindowChanges wc;
	unsigned long wcmask = 0;
	wc.x = w->x; wcmask |= CWX;
	wc.y = w->y; wcmask |= CWY;
	wc.width = w->w; wcmask |= CWWidth;
	wc.height = w->h; wcmask |= CWHeight;
	ret |= XConfigureWindow(g_dpy, w->win, wcmask, &wc);
	ret |= sync();
	return ret;
}

int awosInit() {
	int hasExtensions = 0;
	g_dpy = XOpenDisplay(0);
	if (g_dpy) {
		g_screen = XDefaultScreen(g_dpy);
		g_del = XInternAtom(g_dpy, "WM_DELETE_WINDOW", False);
		hasExtensions = 0 != glXQueryExtension(g_dpy, 0, 0);
	}
#if 1 // !defined(__APPLE__)
	if (hasExtensions)
		glXSwapIntervalSGI = (void*)glXGetProcAddress(
			(GLubyte*)"glXSwapIntervalSGI");
#endif
	return hasExtensions;
}

void awosEnd() {
	XCloseDisplay(g_dpy);
}

aw * awosOpen(const char * t, int x, int y, int width, int height, void * ct) {
	aw * ret = NULL;
	aw * w = calloc(1, sizeof(*ret));
	w->vinfo = chooseVisual(g_dpy, g_screen);
	w->x = x; w->y = y; w->w = width; w->h = height;
	if (w->vinfo) {
		XSetWindowAttributes swa; unsigned long swamask;
		fillWA(&swamask, &swa);
		w->ctx = glXCreateContext(g_dpy, w->vinfo, ct, True);
		w->win = XCreateWindow(g_dpy, XRootWindow(g_dpy, g_screen),
				       x, y, width, height, 
				       0, CopyFromParent,
				       InputOutput, 
				       CopyFromParent,
				       swamask, &swa);
		XSelectInput(g_dpy, w->win, EVMASK);
		XSetWMProtocols(g_dpy, w->win, &g_del, 1);
		// XSetWMProperties
		if (w->win && w->ctx)
			ret = w;
	}
	if (!ret && w)
		awosClose(w);
	if (g_dpy)
		sync();
	return ret;
}

int awosClose(aw * w) {
	glXMakeCurrent(g_dpy, 0, 0);
	if (w->win) awosHide(w);
	if (w->ctx) glXDestroyContext(g_dpy, w->ctx);
	if (w->win) XDestroyWindow(g_dpy, w->win);
	if (w->vinfo) XFree(w->vinfo);
	free(w);
	return 1;
}

int awosSwapBuffers(aw * w) {
	glXSwapBuffers(g_dpy, w->win);
	return 1;
}

int awosPushCurrent(aw * w) {
	w->pushctx = glXGetCurrentContext();
	w->pushwin = glXGetCurrentDrawable();
	return glXMakeCurrent(g_dpy, w->win, w->ctx);
}

int awosPopCurrent(aw * w) {
	return glXMakeCurrent(g_dpy, w->pushwin, w->pushctx);
}

int awosShow(aw * w) {
	int ret = 0; 
	ret |= reconfigure(w);
	ret |= XMapWindow(g_dpy, w->win);
	ret |= sync();
	ret |= reconfigure(w);
	return ret;
}

int awosHide(aw * w) {
	int ret = 0;
	ret |= XUnmapWindow(g_dpy, w->win);
	ret |= sync();
	return ret;
}

int awosSetTitle(aw * w, const char * t) {
	int ret = 0;
	ret |= XStoreName(g_dpy, w->win, t);
	ret |= XSetIconName(g_dpy, w->win, t);
	return ret;
}

int awosMove(aw * w, int x, int y) {
	w->x = x; w->y = y;
	return reconfigure(w);
}

int awosResize(aw * w, int width, int height) {
	w->w = width; w->h = height;
	return reconfigure(w);
}

static int mapButton(int button) {
	int which;
	switch (button) {
	case Button1: which = AW_KEY_MOUSELEFT; break;
	case Button2: which = AW_KEY_MOUSEMIDDLE; break;
	case Button3: which = AW_KEY_MOUSERIGHT; break;
	case Button4: which = AW_KEY_MOUSEWHEELUP; break;
	case Button5: which = AW_KEY_MOUSEWHEELDOWN; break;
	default: which = AW_KEY_NONE;
	}
	return which;
}

static int mapKey(KeyCode keycode) {
	return XKeycodeToKeysym(g_dpy, keycode, 0);
}

static void configure(aw * w, int x, int y, int width, int height) {
	if (x != w->lastx || y != w->lasty)
		got(w, AW_EVENT_MOVE, x, y);
	if (width != w->lastw || height != w->lasth)
		got(w, AW_EVENT_RESIZE, width, height);
	w->lastx = x; w->lasty = y;	w->lastw = width; w->lasth = height;
}

static void handle(aw * w, XEvent * e) {
	switch(e->type) {
	case ClientMessage:
		got(w, AW_EVENT_CLOSE, 0, 0);
		break;
	case ConfigureNotify:
		configure(w, e->xconfigure.x, e->xconfigure.y,
			  e->xconfigure.width, e->xconfigure.height);
		break;
	case ButtonPress:
		got(w, AW_EVENT_DOWN, mapButton(e->xbutton.button), 0);
		break;
	case ButtonRelease:
		got(w, AW_EVENT_UP, mapButton(e->xbutton.button), 0);
		break;
	case MotionNotify:
		got(w, AW_EVENT_MOTION, e->xmotion.x, e->xmotion.y);
		break;
	case KeyPress:
		got(w, AW_EVENT_DOWN, mapKey(e->xkey.keycode), 0);
		break;
	default:
		got(w, AW_EVENT_UNKNOWN, 0, 0);
		break;
	}
}

static int pollEvent(aw * w, XEvent * e) {
	return XCheckWindowEvent(g_dpy, w->win, EVMASK, e)
		|| XCheckTypedWindowEvent(g_dpy, w->win, ClientMessage, e);
}


void awosNextEvent(aw * w) {
	XEvent e;
	if (pollEvent(w, &e))
		handle(w, &e);
	if (w->head != w->tail) {
		w->hdr.next = w->ev[w->tail];
		w->tail++;
		w->tail %= MAX_EVENTS;
	}
}

int awosSetSwapInterval(int i) {
	return setSwapInterval(i);
}

/* 
Local variables: **
c-file-style: "bsd" **
End: **
*/
