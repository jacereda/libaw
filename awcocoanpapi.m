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
	debug("update %p", o);
	[o->l performSelectorOnMainThread:@selector(setNeedsDisplay)
	  withObject:nil waitUntilDone:NO];
	debug("/update %p", o);
}

void awosSetWindow(ins * o, NPWindow * win) {
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
	debug("event %d %d %d", ev, x, y);
}

NPError awosEvent(ins * o, void * ve) {
	extern unsigned mapkeycode(unsigned);
	NPCocoaEvent * e = (NPCocoaEvent *)ve;
	debug("event");
	switch (e->type) {
	case NPCocoaEventDrawRect: 
		debug("NPCocoaEventDrawRect"); 
		break;
	case NPCocoaEventMouseDown: 
		debug("NPCocoaEventMouseDown"); 
		ev(o, AW_EVENT_DOWN, AW_KEY_MOUSELEFT 
		   + e->data.mouse.buttonNumber, 0);
		break;
	case NPCocoaEventMouseUp: 
		debug("NPCocoaEventMouseUp"); 
		ev(o, AW_EVENT_UP, AW_KEY_MOUSELEFT 
		   + e->data.mouse.buttonNumber, 0);
		break;
	case NPCocoaEventMouseMoved: 
		debug("NPCocoaEventMouseMoved"); 
		ev(o, AW_EVENT_MOTION, 
		   e->data.mouse.pluginX, e->data.mouse.pluginY);
		break;
	case NPCocoaEventMouseEntered: 
		debug("NPCocoaEventMouseEntered"); 
		break;
	case NPCocoaEventMouseExited: 
		debug("NPCocoaEventMouseExited"); 
		break;
	case NPCocoaEventMouseDragged: 
		debug("NPCocoaEventMouseDragged"); 
		break;
	case NPCocoaEventKeyDown: 
		debug("NPCocoaEventKeyDown"); 
		ev(o, AW_EVENT_DOWN, mapkeycode(e->data.key.keyCode), 0);
		break;
	case NPCocoaEventKeyUp: 
		debug("NPCocoaEventKeyUp"); 
		ev(o, AW_EVENT_UP, mapkeycode(e->data.key.keyCode), 0);
		break;
	case NPCocoaEventFlagsChanged: 
		debug("NPCocoaEventFlagsChanged"); 
		break;
	case NPCocoaEventFocusChanged: 
		debug("NPCocoaEventFocusChanged"); 
		break;
	case NPCocoaEventWindowFocusChanged: 
		debug("NPCocoaEventWindowFocusChanged"); 
		break;
	case NPCocoaEventScrollWheel: 
		debug("NPCocoaEventScrollWheel"); 
		break;
	case NPCocoaEventTextInput: 
		debug("NPCocoaEventTextInput"); 
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
	debug("os getvalue"); 
	switch(var) {
	case NPNVpluginDrawingModel:
		debug("getval drawingmodel");
		*(int*)v = NPDrawingModelOpenGL;
		break;
	case NPPVpluginCoreAnimationLayer:
		debug("getval layer");
		o->l.backgroundColor = CGColorGetConstantColor(kCGColorBlack);
                o->l.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
		*(CALayer**)v = o->l;
		break;
	default: 
		debug("os getval default"); 
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
	ins * o = (ins*)coData(coCurrent());
	CGLSetCurrentContext(ct);
	debug("drawInCGLContext %p", ct);
	debug("aw>>main");
	coSwitchTo(o->h.coaw);	
	debug("main>>aw");
	CGLSetCurrentContext(ct);
	debug("drawInCGLContext /cocall");
	[super drawInCGLContext: ct pixelFormat: pf
	       forLayerTime: lt displayTime: dt];
	debug("/drawInCGLContext");
	update(o);
}

- (BOOL)canDrawInCGLContext:(CGLContextObj)ct
		pixelFormat:(CGLPixelFormatObj)pf
	       forLayerTime:(CFTimeInterval)lt 
		displayTime:(const CVTimeStamp *)ts
{
	debug("candraw %f", (float)lt);
	ins * o = (ins*)coData(coCurrent());
	BOOL ret = lt - lastdraw > 1 / 60.;
	if (ret)
		lastdraw = lt;
	else
		update(o);
	return ret;
}
@end
