#include <EGL/egl.h>
#include <android_native_app_glue.h>

struct _osg {
	EGLDisplay dpy;
	EGLint w, h;
	struct android_app* app;
};

struct _osc {
	EGLContext ctx;
	EGLint fmt;
};

struct _osw {
	EGLSurface surf;
};

struct _osp {
	int dummy;
};



