#include <stdlib.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/keysym.h>

#include "aw.h"
#include "awos.h"

int setSwapInterval(int interval) {
	return 0 == glXSwapIntervalSGI(interval);
}

#define EVMASK	KeyPressMask | KeyReleaseMask |\
				ButtonPressMask | ButtonReleaseMask |\
				PointerMotionMask | StructureNotifyMask
static Display * g_dpy;
static Atom g_del;
static int g_screen;

struct _aw {
	awHeader hdr;
	Window win;
	GLXContext ctx;
	int x, y, w, h;
	int lastw, lasth;
};

static int sync() {
	return XSync(g_dpy, False);
}

static XVisualInfo * chooseVisual() {
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
//	swa->border_pixel = 0; *swam |= CWBorderPixel;
//	swa->colormap = XDefaultColormap(g_dpy, g_screen); *swam |= CWColormap;
//	swa->win_gravity = StaticGravity; *swam |= CWWinGravity;
//	swa->bit_gravity = StaticGravity; *swam |= CWBitGravity;
}


static Window createWin(int x, int y, int width, int height) {
	XSetWindowAttributes swa; 
	unsigned long swamask;
	XSizeHints hints;
	Window w;
	fillWA(&swamask, &swa);
	w = XCreateWindow(g_dpy, XRootWindow(g_dpy, g_screen),
			  x, y, width, height, 
			  0, CopyFromParent,
			  InputOutput, 
			  CopyFromParent,
			  swamask, &swa);
	XSelectInput(g_dpy, w, EVMASK);
	XSetWMProtocols(g_dpy, w, &g_del, 1);
	hints.flags = USSize | USPosition;
	hints.x = x; hints.y = y;
	hints.width = width; hints.height = height;
	XSetWMNormalHints(g_dpy, w, &hints);
	return w;
}

static int g_bw = -1;
static int g_bh = -1;

static void findBorderSize() {
	XEvent e;
	Window w = createWin(-100, -100, 1, 1);
	XMapWindow(g_dpy, w);
	while (g_bh < 0) {
		XCheckWindowEvent(g_dpy, w, StructureNotifyMask, &e);
		if (e.type == ConfigureNotify 
		    && !e.xconfigure.override_redirect) {
			g_bw = e.xconfigure.x;
			g_bh = e.xconfigure.y;
		}
	}
	XUnmapWindow(g_dpy, w);
	XDestroyWindow(g_dpy, w);
	sync();
}

int awosInit() {
	int hasExtensions = 0;
	g_dpy = XOpenDisplay(0);
	if (g_dpy) {
		g_screen = XDefaultScreen(g_dpy);
		g_del = XInternAtom(g_dpy, "WM_DELETE_WINDOW", False);
		hasExtensions = 0 != glXQueryExtension(g_dpy, 0, 0);
		findBorderSize();
		
	}
//	if (hasExtensions)
//		glXSwapIntervalSGI = (void*)glXGetProcAddress(
//			(GLubyte*)"glXSwapIntervalSGI");
	return hasExtensions;
}

void awosEnd() {
	XCloseDisplay(g_dpy);
}

int awosSetTitle(aw * w, const char * t) {
	return XSetStandardProperties(g_dpy, w->win, t, t, None,
				      (char**)NULL, 0, NULL);
}

static void cleanup(aw * w) {
	if (g_dpy) glXMakeCurrent(g_dpy, 0, 0);
	if (w->ctx) glXDestroyContext(g_dpy, w->ctx);
	if (w->win) XDestroyWindow(g_dpy, w->win);
}

aw * awosOpen(int x, int y, int width, int height, void * ct) {
	aw * ret = NULL;
	aw w;
	XVisualInfo * vinfo = chooseVisual(g_dpy, g_screen);
	w.x = x; w.y = y; w.w = width; w.h = height;
	if (vinfo) {
		w.ctx = glXCreateContext(g_dpy, vinfo, ct, True);
		w.win = createWin(x - g_bw, y - g_bh, width, height);
		XFree(vinfo);
	}
	if (w.ctx && w.win) {
		ret = malloc(sizeof(w));
		*ret = w;
	}
	if (!ret)
		cleanup(&w);
	if (g_dpy)
		sync();
	return ret;
}

int awosClose(aw * w) {
	cleanup(w);
	free(w);
	return 1;
}

int awosSwapBuffers(aw * w) {
	glXSwapBuffers(g_dpy, w->win);
	return 1;
}

int awosMakeCurrent(void * c, void * d) {
	return glXMakeCurrent(g_dpy, (GLXDrawable)d, (GLXContext)c);
}

void * awosGetCurrentContext() {
	return (void*)glXGetCurrentContext();
}

void * awosGetCurrentDrawable() {
	return (void*)glXGetCurrentDrawable();
}

void * awosGetContext(aw * w) {
	return (void*)w->ctx;
}

void * awosGetDrawable(aw * w) {
	return (void*)w->win;
}

int awosShow(aw * w) {
	int ret = 0; 
	ret |= XMapWindow(g_dpy, w->win);
	ret |= sync();
	return ret;
}

int awosHide(aw * w) {
	int ret = 0;
	ret |= XUnmapWindow(g_dpy, w->win);
	ret |= sync();
	return ret;
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


static int translateModifiers(int k) {
	int ret = k;
	switch (k) {
	case XK_Shift_L:
	case XK_Shift_R: ret = AW_KEY_SHIFT; break;
	case XK_Control_L:
	case XK_Control_R: ret = AW_KEY_CONTROL; break;
	case XK_Meta_L:
	case XK_Meta_R: ret = AW_KEY_META; break;
	case XK_Mode_switch:
	case XK_Alt_L:
	case XK_Alt_R: ret = AW_KEY_ALT; break;
	default: ret = k;
	}
	return ret;
}

static int mapKey(KeyCode keycode) {
	return translateModifiers(XKeycodeToKeysym(g_dpy, keycode, 0));
}

static void configure(aw * w, int x, int y, int width, int height) {
	if (width != w->lastw || height != w->lasth)
		got(w, AW_EVENT_RESIZE, width, height);
	w->lastw = width; w->lasth = height;
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
	case KeyRelease:
		got(w, AW_EVENT_UP, mapKey(e->xkey.keycode), 0);
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


void awosPollEvent(aw * w) {
	XEvent e;
	sync();
	if (pollEvent(w, &e))
		handle(w, &e);
}

int awosSetSwapInterval(int i) {
	return setSwapInterval(i);
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
