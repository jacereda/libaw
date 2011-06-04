#include "aw.h"
#include "awos.h"
#include <android/log.h>

#define ASSERT(e) ((e) ? (void)0 : report("%s:%d: ASSERTION FAILED: %s", __FILE__, __LINE__, #e))

void report(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(ANDROID_LOG_ERROR, "aw", fmt, ap);
	va_end(ap);
}

static struct android_app* g_app;

static awkey mapkey(int kc) {
	awkey ret;
	switch (kc) {
//	case AKEYCODE_SOFT_LEFT: ret = ; break;
//	case AKEYCODE_SOFT_RIGHT: ret = ; break;
	case AKEYCODE_HOME: ret = AW_KEY_HOME; break;
//	case AKEYCODE_BACK: ret = ; break;
//	case AKEYCODE_CALL: ret = ; break;
//	case AKEYCODE_ENDCALL: ret = ; break;
	case AKEYCODE_0: ret = AW_KEY_0; break;
	case AKEYCODE_1: ret = AW_KEY_1; break;
	case AKEYCODE_2: ret = AW_KEY_2; break;
	case AKEYCODE_3: ret = AW_KEY_3; break;
	case AKEYCODE_4: ret = AW_KEY_4; break;
	case AKEYCODE_5: ret = AW_KEY_5; break;
	case AKEYCODE_6: ret = AW_KEY_6; break;
	case AKEYCODE_7: ret = AW_KEY_7; break;
	case AKEYCODE_8: ret = AW_KEY_8; break;
	case AKEYCODE_9: ret = AW_KEY_9; break;
	case AKEYCODE_STAR: ret = AW_KEY_KEYPADMULTIPLY; break;
//	case AKEYCODE_POUND: ret = AW_KEY_; break;
	case AKEYCODE_DPAD_UP: ret = AW_KEY_UPARROW; break;
	case AKEYCODE_DPAD_DOWN: ret = AW_KEY_DOWNARROW; break;
	case AKEYCODE_DPAD_LEFT: ret = AW_KEY_LEFTARROW; break;
	case AKEYCODE_DPAD_RIGHT: ret = AW_KEY_RIGHTARROW; break;
	case AKEYCODE_DPAD_CENTER: ret = AW_KEY_CENTER; break;
	case AKEYCODE_VOLUME_UP: ret = AW_KEY_VOLUMEUP; break;
	case AKEYCODE_VOLUME_DOWN: ret = AW_KEY_VOLUMEDOWN; break;
//	case AKEYCODE_POWER: ret = AW_KEY_; break;
	case AKEYCODE_CAMERA: ret = AW_KEY_CAMERA; break;
	case AKEYCODE_CLEAR: ret = AW_KEY_KEYPADCLEAR; break;
	case AKEYCODE_A: ret = AW_KEY_A; break;
	case AKEYCODE_B: ret = AW_KEY_B; break;
	case AKEYCODE_C: ret = AW_KEY_C; break;
	case AKEYCODE_D: ret = AW_KEY_D; break;
	case AKEYCODE_E: ret = AW_KEY_E; break;
	case AKEYCODE_F: ret = AW_KEY_F; break;
	case AKEYCODE_G: ret = AW_KEY_G; break;
	case AKEYCODE_H: ret = AW_KEY_H; break;
	case AKEYCODE_I: ret = AW_KEY_I; break;
	case AKEYCODE_J: ret = AW_KEY_J; break;
	case AKEYCODE_K: ret = AW_KEY_K; break;
	case AKEYCODE_L: ret = AW_KEY_L; break;
	case AKEYCODE_M: ret = AW_KEY_M; break;
	case AKEYCODE_N: ret = AW_KEY_N; break;
	case AKEYCODE_O: ret = AW_KEY_O; break;
	case AKEYCODE_P: ret = AW_KEY_P; break;
	case AKEYCODE_Q: ret = AW_KEY_Q; break;
	case AKEYCODE_R: ret = AW_KEY_R; break;
	case AKEYCODE_S: ret = AW_KEY_S; break;
	case AKEYCODE_T: ret = AW_KEY_T; break;
	case AKEYCODE_U: ret = AW_KEY_U; break;
	case AKEYCODE_V: ret = AW_KEY_V; break;
	case AKEYCODE_W: ret = AW_KEY_W; break;
	case AKEYCODE_X: ret = AW_KEY_X; break;
	case AKEYCODE_Y: ret = AW_KEY_Y; break;
	case AKEYCODE_Z: ret = AW_KEY_Z; break;
	case AKEYCODE_COMMA: ret = AW_KEY_COMMA; break;
	case AKEYCODE_PERIOD: ret = AW_KEY_PERIOD; break;
	case AKEYCODE_ALT_LEFT: ret = AW_KEY_OPTION; break;
	case AKEYCODE_ALT_RIGHT: ret = AW_KEY_RIGHTOPTION; break;
	case AKEYCODE_SHIFT_LEFT: ret = AW_KEY_SHIFT; break;
	case AKEYCODE_SHIFT_RIGHT: ret = AW_KEY_RIGHTSHIFT; break;
	case AKEYCODE_TAB: ret = AW_KEY_TAB; break;
	case AKEYCODE_SPACE: ret = AW_KEY_SPACE; break;
	case AKEYCODE_SYM: ret = AW_KEY_SYM; break;
//	case AKEYCODE_EXPLORER: ret = AW_KEY_; break;
//	case AKEYCODE_ENVELOPE: ret = AW_KEY_; break;
	case AKEYCODE_ENTER: ret = AW_KEY_RETURN; break;
	case AKEYCODE_DEL: ret = AW_KEY_DELETE; break;
	case AKEYCODE_GRAVE: ret = AW_KEY_GRAVE; break;
	case AKEYCODE_MINUS: ret = AW_KEY_MINUS; break;
	case AKEYCODE_EQUALS: ret = AW_KEY_EQUAL; break;
	case AKEYCODE_LEFT_BRACKET: ret = AW_KEY_LEFTBRACKET; break;
	case AKEYCODE_RIGHT_BRACKET: ret = AW_KEY_RIGHTBRACKET; break;
	case AKEYCODE_BACKSLASH: ret = AW_KEY_BACKSLASH; break;
	case AKEYCODE_SEMICOLON: ret = AW_KEY_SEMICOLON; break;
	case AKEYCODE_APOSTROPHE: ret = AW_KEY_QUOTE; break;
	case AKEYCODE_SLASH: ret = AW_KEY_SLASH; break;
//	case AKEYCODE_AT: ret = AW_KEY_; break;
//	case AKEYCODE_NUM: ret = AW_KEY_; break;
//	case AKEYCODE_HEADSETHOOK: ret = AW_KEY_; break;
//	case AKEYCODE_FOCUS: ret = AW_KEY_; break;
	case AKEYCODE_PLUS: ret = AW_KEY_KEYPADPLUS; break;
//	case AKEYCODE_MENU: ret = AW_KEY_; break;
//	case AKEYCODE_NOTIFICATION: ret = AW_KEY_; break;
//	case AKEYCODE_SEARCH: ret = AW_KEY_; break;
//	case AKEYCODE_MEDIA_PLAY_PAUSE: ret = AW_KEY_; break;
//	case AKEYCODE_MEDIA_STOP: ret = AW_KEY_; break;
//	case AKEYCODE_MEDIA_NEXT: ret = AW_KEY_; break;
//	case AKEYCODE_MEDIA_PREVIOUS: ret = AW_KEY_; break;
//	case AKEYCODE_MEDIA_REWIND: ret = AW_KEY_; break;
//	case AKEYCODE_MEDIA_FAST_FORWARD: ret = AW_KEY_; break;
	case AKEYCODE_MUTE: ret = AW_KEY_MUTE; break;
	case AKEYCODE_PAGE_UP: ret = AW_KEY_PAGEUP; break;
	case AKEYCODE_PAGE_DOWN: ret = AW_KEY_PAGEDOWN; break;
//	case AKEYCODE_PICTSYMBOLS: ret = AW_KEY_; break;
//	case AKEYCODE_SWITCH_CHARSET: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_A: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_B: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_C: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_X: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_Y: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_Z: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_L1: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_R1: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_L2: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_R2: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_THUMBL: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_THUMBR: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_START: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_SELECT: ret = AW_KEY_; break;
//	case AKEYCODE_BUTTON_MODE: ret = AW_KEY_; break;
	default:
		ret = AW_KEY_NONE;
	}
	return ret;
}

static int32_t input(struct android_app * app, AInputEvent * e) {
	osw * w = (osw*)app->userData;
	int handled = 1;
	int kc;
	awkey awkc;
	int action;
	if (w) switch (AInputEvent_getType(e)) {
		case AINPUT_EVENT_TYPE_KEY:
			kc = AKeyEvent_getKeyCode(e);
			action = AKeyEvent_getAction(e);
			awkc = mapkey(kc);
			handled = kc != AW_KEY_NONE;
			if (handled) {
				if (action == AKEY_EVENT_ACTION_DOWN)
					got(w, AW_EVENT_DOWN, awkc, 0);
				if (action == AKEY_EVENT_ACTION_UP)
					got(w, AW_EVENT_UP, awkc, 0);
			}
			break;
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
	osw * w = (osw*)app->userData;
	switch (cmd) {
        case APP_CMD_INIT_WINDOW:
		app->userData = app->window;
		break;
        case APP_CMD_TERM_WINDOW:
		debug("got term window");
		got(w, AW_EVENT_CLOSE, 0, 0);
		break;
	}
}

static void pollEvent(struct android_app * app) {
	int ident;
	int events;
        struct android_poll_source* source;
        ident=ALooper_pollAll(0, 0, &events, (void**)&source);
	if (ident >= 0 && source)
		source->process(app, source);
}

void android_main(struct android_app* app) {
	char * argv[] = {"awandroid"};
	app->userData = 0;
	app_dummy();
	app->onAppCmd = handle;
	app->onInputEvent = input;
	debug("wait\n");
	while (!app->userData)
		pollEvent(app);
	ASSERT(app->window);
	g_app = app;
	debug("fakemain\n");
	progrun(1, argv);
	debug("terminating");
}

static void getcfg(osg * g, EGLConfig * cfg) {
	EGLint ncfg;
	const EGLint attr[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	eglChooseConfig(g->dpy, attr, cfg, 1, &ncfg);
	ASSERT(ncfg > 0);
}

static EGLSurface surfnew(osg * g) {
	EGLConfig cfg;
	getcfg(g, &cfg);
	return eglCreateWindowSurface(g->dpy, cfg, g->app->window, 0);	
}

static int surfdel(osg * g, EGLSurface s) {
	return EGL_TRUE == eglDestroySurface(g->dpy, s) ;
}

static EGLContext ctxnew(osg * g) {
	EGLConfig cfg;
	getcfg(g, &cfg);
	return eglCreateContext(g->dpy, cfg, 0, 0);
}

static int ctxdel(osg * g, EGLContext c) {
	return EGL_TRUE == eglDestroyContext(g->dpy, c);
}

int osgInit(osg * g, const char * name) {
	int ok;
	EGLConfig cfg;
	EGLint fmt;
	g->dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	ASSERT(g->dpy);
	g->app = g_app;
	report("g_app: %p", g->app);
	ASSERT(g->app->window);
	ok = eglInitialize(g->dpy, 0, 0) == EGL_TRUE;
	getcfg(g, &cfg);
	eglGetConfigAttrib(g->dpy, cfg, EGL_NATIVE_VISUAL_ID, &fmt);
	ANativeWindow_setBuffersGeometry(g->app->window, 0, 0, fmt);
	if (ok) {
		EGLSurface fakesurf = surfnew(g);
		EGLContext fakectx = ctxnew(g);
		eglMakeCurrent(g->dpy, fakesurf, fakesurf, fakectx);
		eglQuerySurface(g->dpy, fakesurf, EGL_WIDTH, &g->w);
		eglQuerySurface(g->dpy, fakesurf, EGL_HEIGHT, &g->h);
		eglMakeCurrent(g->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, 
			       EGL_NO_CONTEXT);
		ctxdel(g, fakectx);
		surfdel(g, fakesurf);
	}
	return ok;
}

void osgTick(osg * g) {

}

int osgTerm(osg * g) {
	return eglTerminate(g->dpy) == EGL_TRUE;
}

int oswSetTitle(osw * w, const char * t) {
        return 1;
}

int oswInit(osw * w, osg * g, int x, int y, 
	      int width, int height, int bl) {
	EGLContext fakectx;
	report("app %p", g->app);
	g->app->userData = w;
	report("surfnew");
	w->surf = surfnew(g);
	report("/surfnew");
	ASSERT(w->surf);
        return w->surf != 0;
}

int oswTerm(osw * w) {
	return surfdel(wgroup(w), w->surf);
}

int oswSwapBuffers(osw * w) {
	return eglSwapBuffers(wgroup(w)->dpy, w->surf) == EGL_TRUE;
}

int oswMakeCurrent(osw * w, osc * c) {
	return eglMakeCurrent(wgroup(w)->dpy, w->surf, w->surf, c->ctx) == EGL_TRUE;
}

int oswClearCurrent(osw * w) {
        return eglMakeCurrent(wgroup(w)->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, 
			      EGL_NO_CONTEXT) == EGL_TRUE;
}

int oswShow(osw * w) {
        return 1;
}

int oswHide(osw * w) {
        return 1;
}

void oswPollEvent(osw * w) {
	return pollEvent(wgroup(w)->app);
}

int oswSetSwapInterval(osw * w, int si) {
	//return eglSwapInterval(wgroup(w)->dpy, si) == EGL_TRUE;
	return 1;
}

int oscInit(osc * c, osg * g, osc * share) {
	EGLConfig cfg;
	getcfg(g, &cfg);
	c->ctx = ctxnew(g);
        return c->ctx != 0;
}

int oscTerm(osc * c) {
	return ctxdel(cgroup(c), c->ctx);
}

int ospInit(osp * p, const void * rgba, unsigned hotx, unsigned hoty) {
	return 1;
}

int ospTerm(osp * p) {
	return 1;
}

int oswMaximize(osw * w) {
	got(w, AW_EVENT_RESIZE, wgroup(w)->w, wgroup(w)->h);
	return 1;
}

int oswGeometry(osw * w, int x, int y, unsigned width, unsigned height) {
	return oswMaximize(w);
}

void oswPointer(osw * w) {

}

unsigned oswOrder(osw ** w) {
	return 0;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
