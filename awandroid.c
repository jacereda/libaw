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
	aw * w = (aw*)app->userData;
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
	aw * w = (aw*)app->userData;
	switch (cmd) {
        case APP_CMD_INIT_WINDOW:
		g_haswin = app->window != 0;
		break;
        case APP_CMD_TERM_WINDOW:
		debug("got term window");
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
	debug("wait\n");
	while (!g_haswin)
		awosPollEvent(0);
	ASSERT(g_app->window);
	debug("fakemain\n");
	fakemain(1, argv);
	debug("waiting destroy");
	while (!g_app->destroyRequested)
		awosPollEvent(0);
	debug("terminating");
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
