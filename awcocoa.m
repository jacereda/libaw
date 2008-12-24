#include "aw.h"
#include "awos.h"
#include <assert.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSNotification.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSScreen.h>
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

struct _aw {
	awHeader hdr;
	View * view;
	Window * win;
	NSOpenGLContext * ctx;
};

static NSAutoreleasePool * pool;

static void resetPool() {
	[pool release];
	pool = [[NSAutoreleasePool alloc] init];
}

@implementation View

- (void)windowDidResize:(NSNotification *)n {
	NSSize sz = [_w->view frame].size;
	[_w->ctx update];
	got(_w, AW_EVENT_RESIZE, (int)sz.width, (int)sz.height);	
}

- (BOOL) windowShouldClose: (id)s {
	got(_w, AW_EVENT_CLOSE, 0, 0);
	return NO;
}

- (void) handleKeyEvent: (NSEvent *)ev mode: (int) mode {
	NSString * s = [ev characters];
	int sl = [s length];
	int i;
	for (i = 0; i < sl; i++)
		got(_w, mode, [s characterAtIndex: i], 0);
}

- (void) keyUp: (NSEvent *)ev {
	[self handleKeyEvent: ev mode: AW_EVENT_UP];
}

- (void) keyDown: (NSEvent *)ev {
	[self handleKeyEvent: ev mode: AW_EVENT_DOWN];
}

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
	NSOpenGLContext *ctx = 0;
	NSOpenGLPixelFormatAttribute attr[] = {0};
	NSOpenGLPixelFormat * fmt;
	fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
	if (fmt)
		ctx = [[NSOpenGLContext alloc] 
			      initWithFormat:fmt shareContext:share];
	[fmt release];
	return ctx;
}

aw * awosOpen(int x, int y, int width, int height, const char * t, void * ct) {
	NSString * s = [[NSString alloc] initWithUTF8String: t];
	aw * w = 0;
	NSSize scr = [[NSScreen mainScreen] frame].size;
	NSRect rect = NSMakeRect(x, scr.height - y - height, width, height);
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

	[win setContentView: view];
	[win setDelegate: view];
	[win makeFirstResponder: view];
	[win setAcceptsMouseMovedEvents: YES];

	w = calloc(1, sizeof(*w));
	w->view = view;
	w->win = win;
	w->view->_w = w;
	w->win->_w = w;
	w->ctx = ctx;
	[ctx setView: [win contentView]];
	[w->win setTitle: s];
	[s release];
	got(w, AW_EVENT_RESIZE, width, height);
	return w;
}

static NSEvent * nextEvent(Window * win) {
	NSEvent * e = [win nextEventMatchingMask: NSAnyEventMask 
			untilDate: [NSDate distantPast] 
			inMode: NSDefaultRunLoopMode 
			dequeue: YES];
	[NSApp sendEvent: e];
	return e;
}

int awosClose(aw * w, int destroyctx) {
	while (nextEvent(w->win))
		;
	[w->win release];
	if (destroyctx) [w->ctx release];
	[w->view release];
	return 1;
}

int awosSwapBuffers(aw * w) {
	glFlush();
	[w->ctx flushBuffer];
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

void awosPollEvent(aw * w) {
	nextEvent(w->win);
	resetPool();
}

int awosSetSwapInterval(int i) {
        long param = i;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &param);
	return 1;
}

void * awosGetCurrentContext() {
	return [NSOpenGLContext currentContext];
}

void * awosGetCurrentDrawable() {
	return 0;
}

void * awosGetContext(aw * w) {
	return w->ctx;
}

void * awosGetDrawable(aw * w) {
	return w->win;
}

int awosMakeCurrent(void * c, void * d) {
	NSOpenGLContext * cc = (NSOpenGLContext*)c;
	[cc makeCurrentContext];
	return 1;
}

/* 
Local variables: **
c-file-style: "bsd" **
End: **
*/
