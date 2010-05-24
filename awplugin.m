#include <assert.h>
#include <dlfcn.h>
#define NP_NO_CARBON
#include <WebKit/npapi.h>
#include <WebKit/npfunctions.h>
#include <QuartzCore/QuartzCore.h>
#include <pcl.h>

#include "aw.h"
#define report xxx
#include "awos.h"
#undef report
static void report(const char * fmt, ...) {}
#define OSCALL

typedef void (*awmethod)(void);

struct _aw {
        awHeader hdr;
};

struct _ac {
        acHeader hdr;
};

@interface Layer : CAOpenGLLayer {
@public
	aw w;
	ac c;
	NPP instance;
	coroutine_t co;
	CFTimeInterval lastdraw;
}

- (void)drawInCGLContext:(CGLContextObj)ct
	     pixelFormat:(CGLPixelFormatObj)pf
	    forLayerTime:(CFTimeInterval)lt
	     displayTime:(const CVTimeStamp *)dt;

- (BOOL)canDrawInCGLContext:(CGLContextObj)ct
		pixelFormat:(CGLPixelFormatObj)pf
	       forLayerTime:(CFTimeInterval)lt
		displayTime:(const CVTimeStamp *)dt;
@end

typedef Layer obj;

static NPNetscapeFuncs * s_browser = 0;
static NPObject * s_so = 0;

static awmethod resolve(NPIdentifier method) {
	static void * me = 0;
	char * mname = s_browser->utf8fromidentifier(method);
	awmethod ret;
	if (!me)
		me = dlopen(0, 0);
	ret = (awmethod)dlsym(me, mname);
	report("resolve %s %p", mname, ret);
	return ret;
}

static bool hasmethod(NPObject* obj, NPIdentifier method) {
	return resolve(method) != 0;
}

static bool invoke(NPObject* obj, NPIdentifier method, 
		   const NPVariant *args, uint32_t nargs, NPVariant *res) {
	awmethod func = resolve(method);
	if (func)
		func();
	else {
		report("no such method");
		s_browser->setexception(obj, "no such method");
	}
	return func != 0;
}

static bool invokedefault(NPObject *obj, 
			  const NPVariant *args, uint32_t nargs, 
			  NPVariant *res) {
	report("invokedefault");
	return false;
}

static bool hasproperty(NPObject *obj, NPIdentifier prop) {
	report("hasproperty");
	return false;
}

static bool getproperty(NPObject *obj, NPIdentifier prop, NPVariant *res) {
	report("getproperty");
	return false;
}

static NPClass scriptable = {
	NP_CLASS_STRUCT_VERSION,
	NULL,
	NULL,
	NULL,
	hasmethod,
	invoke,
	invokedefault,
	hasproperty,
	getproperty,
	NULL,
	NULL,
};


static void ev(obj * o, int ev, int x, int y) {
	got(&o->w, ev, x, y);
	report("event %d %d %d", ev, x, y);
}

static void entry(void * data) {
	extern int main(int argc, char ** argv);
	int argc = 1;
	char * argv0 = "awplugin";
	main(argc, &argv0);
}




static NPError nnew(NPMIMEType type, NPP i,
		    uint16 mode, int16 argc, char* argn[],
		    char* argv[], NPSavedData* saved) {
	report("new");
	obj * o = [[Layer layer] retain];
	memset(&o->w, 0, sizeof(o->w));
	memset(&o->c, 0, sizeof(o->c));
	i->pdata = o;
	o->instance = i;
        s_browser->setvalue(i, NPPVpluginEventModel, 
			    (void*)NPEventModelCocoa);
	s_browser->setvalue(i, NPNVpluginDrawingModel, 
			    (void *)NPDrawingModelCoreAnimation);
	report("coinit");
	co_thread_init();
	report("cocreate");
	o->co = co_create(entry, o, 0, 8*1024*1024);
	return NPERR_NO_ERROR;
}

static void update(obj * o) {
	[o performSelectorOnMainThread:@selector(setNeedsDisplay)
	   withObject:nil waitUntilDone:NO];
}

static NPError setwindow(NPP i, NPWindow* w) {
	obj * o = (obj*)i->pdata;
	report("%dx%d", w->width, w->height);
	ev(o, AW_EVENT_RESIZE, w->width, w->height);
	update(o);
	return NPERR_NO_ERROR;
}

static NPError destroy(NPP i, NPSavedData **save) {
	obj * o = (obj*)i->pdata;
	report("destroy");
	[o release];
	if(s_so)
		s_browser->releaseobject(s_so);
	return NPERR_NO_ERROR;
}

static NPError getvalue(NPP i, NPPVariable variable, void *v) {
	obj * o = (obj*)i->pdata;
	NPError ret = NPERR_NO_ERROR;
	switch(variable) {
	default: 
		report("getval default"); 
		ret = NPERR_GENERIC_ERROR; 
		break;
	case NPPVpluginNameString: 
		report("getval pluginname"); 
		*(char **)v = "plugin name";
		break;
	case NPPVpluginDescriptionString:
		report("getval plugindesc"); 
		*(char **)v = "plugin description";
		break;
	case NPPVpluginScriptableNPObject:
		report("getval scriptable");
		if(!s_so)
			s_so = s_browser->createobject(i, &scriptable);
		s_browser->retainobject(s_so);
		*(void**)v = s_so;
		break;
	case NPNVpluginDrawingModel:
		report("getval drawingmodel");
		*(int*)v = NPDrawingModelOpenGL;
		break;
	case NPPVpluginCoreAnimationLayer:
		report("getval layer");
		o.backgroundColor = CGColorGetConstantColor(kCGColorBlack);
                o.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
		*(CALayer**)v = o;
		break;
	case NPPVpluginTransparentBool:
		report("getval transp"); // XXX
		break;
	}
	return ret;
}

static NPError event(NPP i, void * ve) {
	extern unsigned mapkeycode(unsigned);
	NPCocoaEvent * e = (NPCocoaEvent *)ve;
	obj * o = (obj*)i->pdata;
	report("event");
	switch (e->type) {
	case NPCocoaEventDrawRect: 
		report("NPCocoaEventDrawRect"); 
		break;
	case NPCocoaEventMouseDown: 
		report("NPCocoaEventMouseDown"); 
		ev(o, AW_EVENT_DOWN, AW_KEY_MOUSELEFT 
		   + e->data.mouse.buttonNumber, 0);
		break;
	case NPCocoaEventMouseUp: 
		report("NPCocoaEventMouseUp"); 
		ev(o, AW_EVENT_UP, AW_KEY_MOUSELEFT 
		   + e->data.mouse.buttonNumber, 0);
		break;
	case NPCocoaEventMouseMoved: 
		report("NPCocoaEventMouseMoved"); 
		ev(o, AW_EVENT_MOTION, 
		   e->data.mouse.pluginX, e->data.mouse.pluginY);
		break;
	case NPCocoaEventMouseEntered: 
		report("NPCocoaEventMouseEntered"); 
		break;
	case NPCocoaEventMouseExited: 
		report("NPCocoaEventMouseExited"); 
		break;
	case NPCocoaEventMouseDragged: 
		report("NPCocoaEventMouseDragged"); 
		break;
	case NPCocoaEventKeyDown: 
		report("NPCocoaEventKeyDown"); 
		ev(o, AW_EVENT_DOWN, mapkeycode(e->data.key.keyCode), 0);
		break;
	case NPCocoaEventKeyUp: 
		report("NPCocoaEventKeyUp"); 
		ev(o, AW_EVENT_UP, mapkeycode(e->data.key.keyCode), 0);
		break;
	case NPCocoaEventFlagsChanged: 
		report("NPCocoaEventFlagsChanged"); 
		break;
	case NPCocoaEventFocusChanged: 
		report("NPCocoaEventFocusChanged"); 
		break;
	case NPCocoaEventWindowFocusChanged: 
		report("NPCocoaEventWindowFocusChanged"); 
		break;
	case NPCocoaEventScrollWheel: 
		report("NPCocoaEventScrollWheel"); 
		break;
	case NPCocoaEventTextInput: 
		report("NPCocoaEventTextInput"); 
		break;
	default: assert(0);
	}
	return NPERR_NO_ERROR;
}

EXPORTED NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* f) {
	report("getentrypoints");
	f->size          = sizeof (NPPluginFuncs);
	f->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	f->newp = nnew;
	f->setwindow = setwindow;
	f->destroy = destroy;
	f->getvalue = getvalue;
	f->event = event;
	return NPERR_NO_ERROR;
}

EXPORTED NPError OSCALL NP_Initialize(NPNetscapeFuncs* f) {
	s_browser = f;
	report("initialize");
#if 0
	if(!f)
		return NPERR_INVALID_FUNCTABLE_ERROR;
	if(((f->version & 0xff00) >> 8) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
#endif
	return NPERR_NO_ERROR;
}

EXPORTED NPError OSCALL NP_Shutdown() {
	report("shutdown");
	return NPERR_NO_ERROR;
}

EXPORTED char * NP_GetMIMEDescription(void) {
	report("getmimedesc");
	return "application/awplugin:.foo:xx@foo.bar";
}

EXPORTED NPError OSCALL NP_GetValue(NPP npp, NPPVariable variable, void *val) {
	report("npgetvalue");
	return getvalue(npp, variable, val);
}

int awosInit() { 
	report("awosInit");
	return 1; 
}

int awosEnd() { 
	report("awosEnd");
	return 1; 
}

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
	obj * o = (obj*)co_get_data(co_current());
	report("awosOpen");
	return &o->w;
}

int awosSetTitle(aw * w, const char * t) {
	report("awosSetTitle");
	return 1;
}

int awosClose(aw * w) {
	report("awosClose");
	return 1;
}

int awosMakeCurrent(aw * w, ac * c) {
//	obj * o = (obj*)co_get_data(co_current());
	report("awosMakeCurrent");
	return 1;
}

int awosClearCurrent(aw * w) {
	report("awosClearCurrent");
	return 1;
}

int awosSwapBuffers(aw * w) {
	obj * o = (obj*)co_get_data(co_current());
	report("awosSwapBuffers");
	update(o);
	co_resume();
	report("/awosSwapBuffers");
	return 1;
}

int awosShow(aw * w) {
	report("awosShow");
	return 1;
}

int awosHide(aw * w) {
	report("awosHide");
	return 1;
}

void awosPollEvent(aw * w) {
	report("awosPollEvent");
}

int awosSetSwapInterval(aw * w, int interval) {
	report("awosSwapInterval");
	GLint param = interval;
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &param);
	return 1;
}

int acosInit() {
	report("acosInit");
	return 1;
}
int acosEnd() {
	report("acosEnd");
	return 1;
}

ac * acosNew(ac * c) {
	obj * o = (obj*)co_get_data(co_current());
	report("acosNew");
//	co_resume();
	return &o->c;
}

int acosDel(ac * c) {
	report("acosDel");
	return 1;
}

@implementation Layer
- (void)drawInCGLContext:(CGLContextObj)ct
	     pixelFormat:(CGLPixelFormatObj)pf
	    forLayerTime:(CFTimeInterval)lt
	     displayTime:(const CVTimeStamp *)dt
{
	CGLSetCurrentContext(ct);
//	report("drawInCGLContext cocall");
	co_call(co);
	CGLSetCurrentContext(ct);
	report("drawInCGLContext /cocall");
	[super drawInCGLContext: ct pixelFormat: pf
	       forLayerTime: lt displayTime: dt];
}

- (BOOL)canDrawInCGLContext:(CGLContextObj)ct
		pixelFormat:(CGLPixelFormatObj)pf
	       forLayerTime:(CFTimeInterval)lt 
		displayTime:(const CVTimeStamp *)ts
{
//	report("candraw %f", (float)lt);
	BOOL ret = lt - lastdraw > 1 / 60.;
	if (ret)
		lastdraw = lt;
	else
		update(self);
	return ret;
}
@end


const char * awResourcesPath() {
	obj * o = (obj*)co_get_data(co_current());
	return [[[NSBundle bundleForClass: [o class]] 
			resourcePath] UTF8String];
}
