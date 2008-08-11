#include "aw.h"
#include "awos.h"
#include <assert.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSNotification.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSEvent.h>
#include <AppKit/NSOpenGL.h>
#include <AppKit/NSView.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

@interface Window : NSWindow {
	aw * _w;
}
@end

@interface View : NSView {
	aw * _w;
}
@end

@interface Delegate : NSObject {
	aw * _w;
}
@end

struct _aw {
	awHeader hdr;
	View * view;
	Window * win;
	Delegate * delegate;
	NSOpenGLContext * ctx;
	NSOpenGLContext * pushctx;
};

static NSAutoreleasePool * pool;

static void resetPool() {
	[pool release];
	pool = [[NSAutoreleasePool alloc] init];
}

@implementation View
- (void) handleMouseEvent: (NSEvent *)ev mode: (int)mode{
	got(_w, mode, [ev buttonNumber] + AW_KEY_MOUSELEFT, 0);
}

- (void) mouseDown: (NSEvent*)ev {
	[self handleMouseEvent: ev mode: AW_EVENT_DOWN];
}

- (void) mouseUp: (NSEvent*)ev {
	[self handleMouseEvent: ev mode: AW_EVENT_UP];
}

- (void) handleMotion: (NSEvent *)ev {
	NSPoint l = [ev locationInWindow];
	got(_w, AW_EVENT_MOTION, l.x, [self frame].size.height - l.y);
}

- (void) mouseDragged: (NSEvent *)ev {
	[self handleMotion: ev];
}

- (void) mouseMoved: (NSEvent *)ev {
	[self handleMotion: ev];
}

- (BOOL) isOpaque {
	return YES;
}
@end

@implementation Delegate
- (void)windowDidResize:(NSNotification *)n {
	NSSize sz = [_w->view frame].size;
	[_w->ctx update];
	report("window did resize: %d %d", (int)sz.width, (int)sz.height);
	got(_w, AW_EVENT_RESIZE, (int)sz.width, (int)sz.height);	
}

- (void) handleKeyEvent: (NSEvent *)ev mode: (int) mode {
	NSString * s = [ev characters];
	int sl = [s length];
	int i;
	for (i = 0; i < sl; i++)
		got(_w, AW_EVENT_UP, [s characterAtIndex: i], 0);
}

- (void) keyUp: (NSEvent *)ev {
	[self handleKeyEvent: ev mode: AW_EVENT_UP];
}

- (void) keyDown: (NSEvent *)ev {
	[self handleKeyEvent: ev mode: AW_EVENT_DOWN];
}

- (BOOL) windowShouldClose: (id)s {
	got(_w, AW_EVENT_CLOSE, 0, 0);
	return NO;
}
@end

@implementation Window
@end


int awosInit() {
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	TransformProcessType(&psn, kProcessTransformToForegroundApplication);
	SetFrontProcess(&psn);
	pool = [[NSAutoreleasePool alloc] init];
	NSApp = [NSApplication sharedApplication];
	[NSApp finishLaunching];
	[NSApp activateIgnoringOtherApps: YES];
	return 1;
}

void awosEnd() {
	[NSApp release];
	[pool release];
}

static NSOpenGLContext * createContext(NSOpenGLContext * share) {
	NSOpenGLContext *ctx;
	NSOpenGLPixelFormatAttribute attr[] = {0};
	NSOpenGLPixelFormat * fmt;
	fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
	if (fmt)
		ctx = [[NSOpenGLContext alloc] 
			      initWithFormat:fmt shareContext:share];
	[fmt release];
	return ctx;
}

aw * awosOpen(const char * t, int x, int y, int width, int height, void * ct) {
	aw * w = 0;
	NSRect rect = NSMakeRect(x, y, width, height);
	unsigned int style = 0
		| NSTitledWindowMask
		| NSClosableWindowMask
		| NSMiniaturizableWindowMask 
		| NSResizableWindowMask
		;
	Window *win = [[Window alloc] initWithContentRect: rect 
				      styleMask: style
				      backing: NSBackingStoreBuffered
				      defer:NO];
	NSOpenGLContext * ctx = createContext((NSOpenGLContext*)ct);
	NSRect frm = [Window contentRectForFrameRect: [win frame]
			     styleMask: style];
	View * view = [[View alloc] initWithFrame: frm];
	Delegate * delegate = [[Delegate alloc] init];

	[win setContentView: view];
	[win setDelegate: delegate];
	[win makeFirstResponder: view];
	[win setAcceptsMouseMovedEvents: YES];

	w = calloc(1, sizeof(*w));
	w->view = view;
	w->win = win;
	w->delegate = delegate;
	w->view->_w = w;
	w->win->_w = w;
	w->delegate->_w = w;
	w->ctx = ctx;
	[ctx setView: [win contentView]];
	return w;
}

int awosClose(aw * w) {
	[w->win release];
	[w->view release];
	[w->ctx release];
	return 1;
}

int awosSwapBuffers(aw * w) {
	glFlush();
	[w->ctx flushBuffer];
	return 1;
}

int awosPushCurrent(aw * w) {
	assert(w->pushctx == 0);
	w->pushctx = [NSOpenGLContext currentContext];
	report("%p %p", [w->ctx view], w->view);
	[w->ctx makeCurrentContext];
	return 1;
}

int awosPopCurrent(aw * w) {
	[w->pushctx makeCurrentContext];
	w->pushctx = 0;
	return 1;
}

int awosShow(aw * w) {
	[w->win makeKeyAndOrderFront: w->view];
	return 1;
}

int awosHide(aw * w) {
	[w->win orderOut: nil];
	return 1;
}

int awosSetTitle(aw * w, const char * t) {
	NSString * s = [[NSString alloc] initWithUTF8String: t];
	[w->win setTitle: s];
	[s release];
	return 1;
}

int awosResize(aw * w, int width, int height) {
	NSSize sz;
	sz.width = width;  sz.height = height;
	[w->win setContentSize: sz];
	return 1;
}

void awosPollEvent(aw * w) {
	[NSApp sendEvent: [w->win nextEventMatchingMask: NSAnyEventMask 
			    untilDate: [NSDate distantPast] 
			    inMode: NSDefaultRunLoopMode 
			    dequeue: YES]];
	resetPool();
}

int awosSetSwapInterval(int i) {
        long param = i;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &param);
	return 1;
}

/* 
Local variables: **
c-file-style: "bsd" **
End: **
*/
