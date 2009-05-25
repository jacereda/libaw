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
@public
	aw * _w;
	ac * _c;
}
@end

@interface View : NSView {
	unsigned _prevflags;
@public
	aw * _w;
}
@end

struct _aw {
	awHeader hdr;
	View * view;
	Window * win;
};

struct _ac {
	acHeader hdr;
	NSOpenGLContext * ctx;
};

static NSAutoreleasePool * g_apppool;
static NSAutoreleasePool * g_pool;

static void resetPool() {
	[g_pool release];
	g_pool = [[NSAutoreleasePool alloc] init];
}

@implementation View

- (void)windowDidResize:(NSNotification *)n {
	NSSize sz = [_w->view frame].size;
	if (_w->hdr.ctx->ctx) [_w->hdr.ctx->ctx update];
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

- (unsigned) keyFor: (unsigned)mask {
	unsigned ret = 0;
	switch (mask) {
	case NSShiftKeyMask: ret = AW_KEY_SHIFT; break;
	case NSControlKeyMask: ret = AW_KEY_CONTROL; break;
	case NSAlternateKeyMask: ret = AW_KEY_ALT; break;
	case NSCommandKeyMask: ret = AW_KEY_META; break;
	default: assert(0);
	}
	return ret;
}

- (void) handleMod: (unsigned)mask 
	     flags: (unsigned)flags {
	unsigned delta = flags ^ _prevflags;
	unsigned pressed = delta & flags;
	unsigned released = delta & ~flags;
	if (mask & pressed)
		got(_w, AW_EVENT_DOWN, [self keyFor: mask], 0);
	if (mask & released)
		got(_w, AW_EVENT_UP, [self keyFor: mask], 0);
}

- (void) flagsChanged: (NSEvent *)ev {
	unsigned flags = [ev modifierFlags];
	[self handleMod: NSShiftKeyMask flags: flags];
	[self handleMod: NSControlKeyMask flags: flags];
	[self handleMod: NSAlternateKeyMask flags: flags];
	[self handleMod: NSCommandKeyMask flags: flags];
	_prevflags = flags;
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
	g_apppool = [[NSAutoreleasePool alloc] init];
	NSApp = [NSApplication sharedApplication];
	g_pool = [[NSAutoreleasePool alloc] init];
	[NSApp finishLaunching];
	[NSApp activateIgnoringOtherApps: YES];
	return 1;
}

int awosEnd() {
	[g_pool release];
	[NSApp release];
	[g_apppool release];
	return 1;
}

int awosSetTitle(aw * w, const char * t) {
	NSString * s = [[NSString alloc] initWithUTF8String: t];
	[w->win setTitle: s];
	[s release];
	return 1;
}

aw * awosOpen(int x, int y, int width, int height) {
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

int awosClose(aw * w) {
	while (nextEvent(w->win))
		;
	[w->win release];
	[w->view release];
	return 1;
}

int awosSwapBuffers(aw * w) {
	glFlush();
	if (w->hdr.ctx->ctx) [w->hdr.ctx->ctx flushBuffer];
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

int awosMakeCurrent(aw * w) {
	if (w->hdr.ctx) {
		[w->hdr.ctx->ctx setView: [w->win contentView]];
		[w->hdr.ctx->ctx makeCurrentContext];
	}
	else
		[NSOpenGLContext clearCurrentContext];
	return 1;
}

int acosInit() {
	return 1;
}

int acosEnd() {
	return 1;
}

ac * acosNew(ac * share) {
	ac * c = 0;
	NSOpenGLContext *ctx = 0;
	NSOpenGLPixelFormatAttribute attr[] = {0};
	NSOpenGLPixelFormat * fmt;
	fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
	if (fmt)
		ctx = [[NSOpenGLContext alloc] 
			      initWithFormat:fmt 
			      shareContext: share? share->ctx : 0];
	[fmt release];
	if (ctx)
		c = malloc(sizeof(*c));
	if (c)
		c->ctx = ctx;
	return c;
}

int acosDel(ac * c) {
	[c->ctx release];
	return 1;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
