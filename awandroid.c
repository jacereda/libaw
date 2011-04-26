#include "aw.h"
#include "awos.h"
#include <EGL/egl.h>
#include <android/log.h>
#include <android_native_app_glue.h>

static int g_haswin;

#define ASSERT(e) ((e) ? (void)0 : report("%s:%d: ASSERTION FAILED: %s", __FILE__, __LINE__, #e))

void report(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(ANDROID_LOG_ERROR, "aw", fmt, ap);
	va_end(ap);
}

struct _ac {
	acHeader hdr;
	EGLContext ctx;
	EGLint fmt;
};

struct _aw {
	awHeader hdr;
	EGLSurface surf;
};

static EGLDisplay g_dpy;
static struct android_app* g_app;

static int32_t input(struct android_app * app, AInputEvent * e) {
	aw * w = (aw*)app->userData;
	int handled = 1;
	if (w) switch (AInputEvent_getType(e)) {
	case AINPUT_EVENT_TYPE_MOTION:
		got(w, AW_EVENT_MOTION, 
		    AMotionEvent_getX(e, 0), AMotionEvent_getY(e, 0));
		break;
	default:
		handled = 0;
	}
	return handled;
}

static void handle(struct android_app* app, int32_t cmd) {
	aw * w = (aw*)app->userData;
	switch (cmd) {
        case APP_CMD_INIT_WINDOW:
		g_haswin = app->window != 0;
		break;
        case APP_CMD_TERM_WINDOW:
		report("got term window");
		got(w, AW_EVENT_CLOSE, 0, 0);
		break;
	}
}

void android_main(struct android_app* state) {
	char * argv[] = {"awandroid"};
	g_app = state;
	g_haswin = 0;
	app_dummy();
	g_app->onAppCmd = handle;
	g_app->onInputEvent = input;
	report("wait\n");
	while (!g_haswin)
		awosPollEvent(0);
	ASSERT(g_app->window);
	report("fakemain\n");
	fakemain(1, argv);
	report("waiting destroy");
	while (!g_app->destroyRequested)
		awosPollEvent(0);
	report("terminating");
}

int awosInit() {
	g_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	ASSERT(g_dpy);
	return eglInitialize(g_dpy, 0, 0) == EGL_TRUE;
}

int awosEnd() {
        return eglTerminate(g_dpy) == EGL_TRUE;
}

int awosSetTitle(aw * w, const char * t) {
        return 1;
}

static void getcfg(EGLConfig * cfg) {
	EGLint ncfg;
	const EGLint attr[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	eglChooseConfig(g_dpy, attr, cfg, 1, &ncfg);
	ASSERT(ncfg > 0);
}

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
	aw * w = calloc(1, sizeof(*w));
	EGLint ww=0, wh=0;
	EGLConfig cfg;
	EGLint fmt;
	EGLContext fakectx;
	g_app->userData = w;
	getcfg(&cfg);
	ASSERT(g_app->window);
	ASSERT(g_dpy);
	eglGetConfigAttrib(g_dpy, cfg, EGL_NATIVE_VISUAL_ID, &fmt);
	ANativeWindow_setBuffersGeometry(g_app->window, 0, 0, fmt);
	w->surf = eglCreateWindowSurface(g_dpy, cfg, g_app->window, 0);
	ASSERT(w->surf);
	fakectx = eglCreateContext(g_dpy, cfg, 0, 0);
	eglMakeCurrent(g_dpy, w->surf, w->surf, fakectx);
	eglQuerySurface(g_dpy, w->surf, EGL_WIDTH, &ww);
	eglQuerySurface(g_dpy, w->surf, EGL_HEIGHT, &wh);
	eglMakeCurrent(g_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, 
		       EGL_NO_CONTEXT);
	eglDestroyContext(g_dpy, fakectx);
	got(w, AW_EVENT_RESIZE, ww, wh);
        return w;
}

int awosClose(aw * w) {
	return eglDestroySurface(g_dpy, w->surf) == EGL_TRUE;
}

int awosSwapBuffers(aw * w) {
	return eglSwapBuffers(g_dpy, w->surf) == EGL_TRUE;
}

int awosMakeCurrent(aw * w, ac * c) {
	return eglMakeCurrent(g_dpy, w->surf, w->surf, c->ctx) == EGL_TRUE;
}

int awosClearCurrent(aw * w) {
        return eglMakeCurrent(g_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, 
			      EGL_NO_CONTEXT) == EGL_TRUE;
}

int awosShow(aw * w) {
        return 1;
}

int awosHide(aw * w) {
        return 1;
}

void awosPollEvent(aw * w) {
	int ident;
	int events;
        struct android_poll_source* source;
        ident=ALooper_pollAll(0, NULL, &events, (void**)&source);
	if (ident >= 0 && source)
		source->process(g_app, source);
}

int awosSetSwapInterval(aw * w, int si) {
	//return eglSwapInterval(g_dpy, si) == EGL_TRUE;
	return 1;
}

ac * acosNew(ac * share) {
	ac * c = calloc(1, sizeof(*c));
	EGLConfig cfg;
	getcfg(&cfg);
	c->ctx = eglCreateContext(g_dpy, cfg, NULL, NULL);
        return c;
}

int acosDel(ac * c) {
	return eglDestroyContext(g_dpy, c->ctx) == EGL_TRUE;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
