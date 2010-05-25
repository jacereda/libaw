#include "awnpapios.h"
#include <stdlib.h>

@interface Layer : CAOpenGLLayer {
@public
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


struct _ins {
	insHeader h;
	Layer * l;
};

ins * awosNew(NPNetscapeFuncs * browser, NPP i) {
	ins * ret = calloc(1, sizeof(ins));
	ret->l = [[Layer layer] retain];
        browser->setvalue(i, NPPVpluginEventModel, 
			  (void *)NPEventModelCocoa);
	browser->setvalue(i, NPNVpluginDrawingModel, 
			  (void *)NPDrawingModelCoreAnimation);
	return ret;
}

void awosDel(ins * o) {
	[o->l release];
	free(o);
}

static void update(ins * o) {
	report("update %p", o);
	[o->l performSelectorOnMainThread:@selector(setNeedsDisplay)
	  withObject:nil waitUntilDone:NO];
	report("/update %p", o);
}

void awosSetWindow(ins * o, void * vwin) {
	update(o);
}

int awosMakeCurrentI(ins * o) {
	return 1;
}

int awosClearCurrentI(ins * o) {
        return 1;
}

void awosUpdate(ins * o) {
	update(o);
}

static void ev(ins * o, int ev, int x, int y) {
	got(&o->h.w, ev, x, y);
	report("event %d %d %d", ev, x, y);
}

NPError awosEvent(ins * o, void * ve) {
	extern unsigned mapkeycode(unsigned);
	NPCocoaEvent * e = (NPCocoaEvent *)ve;
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

const char * awosResourcesPath(ins * o) {
	return [[[NSBundle bundleForClass: [o->l class]] 
			resourcePath] UTF8String];
}

NPError awosGetValue(NPP i, NPPVariable var, void * v) {
	ins * o = (ins*)i->pdata;
	NPError ret = NPERR_NO_ERROR;
	report("os getvalue"); 
	switch(var) {
	case NPNVpluginDrawingModel:
		report("getval drawingmodel");
		*(int*)v = NPDrawingModelOpenGL;
		break;
	case NPPVpluginCoreAnimationLayer:
		report("getval layer");
		o->l.backgroundColor = CGColorGetConstantColor(kCGColorBlack);
                o->l.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
		*(CALayer**)v = o->l;
		break;
	default: 
		report("os getval default"); 
		ret = NPERR_GENERIC_ERROR; 
		break;
	}
	return ret;
} 

@implementation Layer
- (void)drawInCGLContext:(CGLContextObj)ct
	     pixelFormat:(CGLPixelFormatObj)pf
	    forLayerTime:(CFTimeInterval)lt
	     displayTime:(const CVTimeStamp *)dt
{
	static int i;
	ins * o = (ins*)coData(coCurrent());
	CGLSetCurrentContext(ct);
	report("drawInCGLContext %p", ct);
	report("aw>>main");
	coSwitchTo(o->h.coaw);	
//	glClearColor((i++ & 0xff) / 255.0, 0, 1, 0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	report("main>>aw");
	CGLSetCurrentContext(ct);
	report("drawInCGLContext /cocall");
	[super drawInCGLContext: ct pixelFormat: pf
	       forLayerTime: lt displayTime: dt];
	report("/drawInCGLContext");
	update(o);
}

- (BOOL)canDrawInCGLContext:(CGLContextObj)ct
		pixelFormat:(CGLPixelFormatObj)pf
	       forLayerTime:(CFTimeInterval)lt 
		displayTime:(const CVTimeStamp *)ts
{
	report("candraw %f", (float)lt);
	ins * o = (ins*)coData(coCurrent());
	BOOL ret = lt - lastdraw > 1 / 60.;
	if (ret)
		lastdraw = lt;
	else
		update(o);
	return ret;
}
@end
