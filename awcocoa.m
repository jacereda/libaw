/*
  Copyright (c) 2008-2009, Jorge Acereda Maciá
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:
        
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
        
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the author nor the names of its contributors
  may be used to endorse or promote products derived from this
  software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <assert.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSNotification.h>
#include <AppKit/NSApplicationScripting.h>
#include <AppKit/NSScreen.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSPasteboard.h>
#include <AppKit/NSBitmapImageRep.h>
#include <AppKit/NSImage.h>
#include <AppKit/NSTrackingArea.h>
#include <AppKit/NSCursor.h>
#include <AppKit/NSEvent.h>
#include <AppKit/NSOpenGL.h>
#include <AppKit/NSTextView.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

@class View;
@class Window;
#include "aw.h"
#include "awos.h"

@interface AppDelegate : NSObject {
}
@end

@interface Window : NSWindow {
@public
        osw * _w;
}
@end

@interface View : NSTextView<NSWindowDelegate> {
        unsigned _prevflags;
        BOOL _dragging;
        NSTrackingArea * _currta;
@public
        osw * _w;
}
- (NSPoint)toAbs: (NSPoint)p;
- (void) handleMove;
@end


static int reversed(int y) {
        NSSize ss = [[NSScreen mainScreen] frame].size;
        return ss.height - y - 1;
}

static NSRect fromQuartz(NSRect r) {
        NSSize ss = [[NSScreen mainScreen] frame].size;
        return NSMakeRect(r.origin.x, 
                          ss.height - r.origin.y - r.size.height,
                          r.size.width, 
                          r.size.height);
}

static NSRect toQuartz(NSRect r) {
        return fromQuartz(r);
}

static NSCursor * currentCursor(osw * w) {
        osp * p = wpointer(w);
        return p? p->cur : w->defcur;
}

@implementation AppDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed: (NSApplication *)a {
        return NO;
}

- (void)timerFired: (id)s {
        dispatch();
        if (progfinished())
                [NSApp terminate: nil];
}

- (void)applicationDidFinishLaunching: (NSNotification *)n {
        NSTimer * t = [NSTimer timerWithTimeInterval: 0.001
                                              target: self
                                            selector: @selector(timerFired:)
                                            userInfo: nil
                                             repeats: YES];
        [[NSRunLoop currentRunLoop] addTimer: t
                                     forMode: NSDefaultRunLoopMode];
        //Ensure timer fires during resize        
        [[NSRunLoop currentRunLoop] addTimer: t
                                     forMode: NSEventTrackingRunLoopMode];
}
@end

@implementation View

- (void) dealloc {
        _w->_vfreed = 1;
        [super dealloc];
}

- (void)establishCursor {
        NSCursor * cur = currentCursor(_w);
        [cur set];
}

- (void)cursorUpdate: (NSEvent*)e {
        [super cursorUpdate: e];
        [self establishCursor];

}

- (NSPoint)toAbs: (NSPoint)p {
        return [_w->win convertBaseToScreen: p];
}

- (void)setConstrainedFrameSize:(NSSize)desiredSize{}


- (BOOL) acceptsFirstResponder {
        return YES;
}

- (void) handleMove {
        NSSize sz = [self frame].size;
        NSPoint ul = NSMakePoint(0, sz.height - 1);
        NSPoint abs = [self toAbs: ul];
        got(_w, AW_EVENT_POSITION, abs.x, reversed(abs.y));
}

- (void) windowDidMove:(NSNotification *)n {
        [self handleMove];
}

- (void) windowDidResize:(NSNotification *)n {
        NSRect fr = [_w->view frame];
        NSSize sz = fr.size;
        osc * c = wcontext(_w);
        if (c) [c->ctx update];
        got(_w, AW_EVENT_RESIZE, sz.width, sz.height);
        [self handleMove];
}

- (void) windowDidExpose:(NSNotification *)n {
        got(_w, AW_EVENT_EXPOSED, 0, 0);
}

- (void) windowDidResignKey:(NSNotification *)n {
        got(_w, AW_EVENT_KILLFOCUS, 0, 0);
}

- (BOOL) windowShouldClose: (id)s {
        got(_w, AW_EVENT_CLOSE, 0, 0);
        return NO;
}

extern unsigned mapkeycode(unsigned);

- (void) handleKeyEvent: (NSEvent *)ev mode: (int) mode {
        got(_w, mode, mapkeycode([ev keyCode]), 0);
}

- (void) keyUp: (NSEvent *)ev {
        [self handleKeyEvent: ev mode: AW_EVENT_UP];
}

- (void) keyDown: (NSEvent *)ev {
        [self handleKeyEvent: ev mode: AW_EVENT_DOWN];
        [self interpretKeyEvents: [NSArray arrayWithObject: ev]];
}

- (void)deleteBackward:(id)sender {
        got(_w, AW_EVENT_UNICODE, AW_KEY_DELETE, 0);
}

- (void)insertText:(id)s {
        int sl = [s length];
        int i;
        for (i = 0; i < sl; i++)
                got(_w, AW_EVENT_UNICODE, [s characterAtIndex: i], 0);
}

- (unsigned) keyFor: (unsigned)mask {
        unsigned ret = 0;
        switch (mask) {
        case NSShiftKeyMask: ret = AW_KEY_SHIFT; break;
        case NSControlKeyMask: ret = AW_KEY_CONTROL; break;
        case NSAlternateKeyMask: ret = AW_KEY_OPTION; break;
        case NSCommandKeyMask: ret = AW_KEY_COMMAND; break;
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
        [super flagsChanged: ev];
}

- (void) handleMouseEvent: (NSEvent *)ev mode: (int)mode{
        got(_w, mode, [ev buttonNumber] + AW_KEY_MOUSELEFT, 0);
}

- (void) mouseDown: (NSEvent*)ev {
        _dragging = YES;
        [self lockFocus];
        [self handleMouseEvent: ev mode: AW_EVENT_DOWN];
        [self updateTrackingAreas];
//        [super mouseDown: ev];
}

- (void) mouseUp: (NSEvent*)ev {
        _dragging = NO;
        [self handleMouseEvent: ev mode: AW_EVENT_UP];
        [self updateTrackingAreas];
        [self unlockFocus];
//        [super mouseUp: ev];
}

- (void) rightMouseDown: (NSEvent*)ev {
        [self handleMouseEvent: ev mode: AW_EVENT_DOWN];
}

- (void) rightMouseUp: (NSEvent*)ev {
        [self handleMouseEvent: ev mode: AW_EVENT_UP];
}

- (void) otherMouseDown: (NSEvent*)ev {
        [self handleMouseEvent: ev mode: AW_EVENT_DOWN];
}

- (void) otherMouseUp: (NSEvent*)ev {
        [self handleMouseEvent: ev mode: AW_EVENT_UP];
}

- (void)scrollWheel:(NSEvent *)ev {
        unsigned k = [ev deltaY] > 0? 
                AW_KEY_MOUSEWHEELUP : AW_KEY_MOUSEWHEELDOWN;
        got(_w, AW_EVENT_DOWN, k, 0);
        got(_w, AW_EVENT_UP, k, 0);
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

- (void) mouseExited: (NSEvent *)ev {
}

- (void) mouseEntered: (NSEvent *)ev {
}

- (void)updateTrackingAreas {
        NSRect rect;
        [self removeTrackingArea: _currta];
        [_currta release];
        _currta = 0;
        if (!_dragging) {
                rect = [self visibleRect];
                _currta = [[NSTrackingArea alloc] 
                                  initWithRect: rect
                                       options: 0
                                  | NSTrackingActiveWhenFirstResponder
//                                     | NSTrackingActiveAlways
                                  | NSTrackingCursorUpdate
//                                          | NSTrackingMouseEnteredAndExited
//                                          | NSTrackingInVisibleRect
//                                          | NSTrackingEnabledDuringMouseDrag
//                                          | NSTrackingAssumeInside
                                         owner: self
                                      userInfo: nil];
                [self addTrackingArea: _currta];
        }
        [super updateTrackingAreas];
}

- (BOOL) isOpaque {
        return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
        NSPasteboard * pb = [sender draggingPasteboard];
        NSArray * a = [pb propertyListForType:NSFilenamesPboardType];
        unsigned i;
        for (i = 0; i < [a count]; i++) {
                NSString * s = [a objectAtIndex: i];
                got(_w, AW_EVENT_DROP, (uintptr_t)strdup([s UTF8String]), 0);
        }
        return YES;
}

- (void)display {
        assert(0);
}

- (void)drawRect:(NSRect)dirtyRect {
        assert(0);
}

- (BOOL)canDraw { return NO; }

@end

@implementation Window

- (void) dealloc {
        _w->_wfreed = 1;
        [super dealloc];
}

- (BOOL) canBecomeKeyWindow
{
        return YES;
}

- (BOOL)isReleasedWhenClosed
{
        return NO;
}

- (void)display {
        assert(0);
}

@end

int oswSetTitle(osw * w, const char * t) {
        NSString * s = [[NSString alloc] initWithUTF8String: t];
        [w->win setTitle: s];
        [s release];
        return 1;
}

static void openwin(osw * w, int x, int y, 
                    int width, int height, unsigned style) {
        Window * win;
        NSRect frm;
        View * view;
        NSRect rect = toQuartz(NSMakeRect(x, y, width, height));
        win = [[Window alloc] initWithContentRect: rect 
                              styleMask: style
                              backing: NSBackingStoreBuffered
                              defer:NO];
        frm = [Window contentRectForFrameRect: [win frame] styleMask: style];
        view = [[View alloc] initWithFrame: frm];
        w->view = view;
        w->win = win;
        w->view->_w = w;
        w->win->_w = w;
        w->defcur = [[NSCursor arrowCursor] retain];
        [win makeFirstResponder: view];
        [win setDelegate: view];
        [win setContentView: view];
        [view setAutoresizesSubviews:YES];
        [view registerForDraggedTypes: [NSArray arrayWithObject:NSFilenamesPboardType]];

        [win setAcceptsMouseMovedEvents: YES];
        [win setReleasedWhenClosed: NO];
        [view updateTrackingAreas];
}

int oswInit(osw * w, osg * g, int x, int y, 
            int width, int height, int bl) {
        unsigned style = 0;
        if (!bl) {
                style += 0
                        | NSTitledWindowMask
                        | NSClosableWindowMask
                        | NSMiniaturizableWindowMask 
                        | NSResizableWindowMask
                        ;
        }
        openwin(w, x, y, width, height, style);
        if (bl)
                [w->win setLevel: NSPopUpMenuWindowLevel];
        return 1;
}

int oswTerm(osw * w) {
        [w->win makeKeyAndOrderFront: nil];
        [w->win makeFirstResponder: nil];
        [w->win setDelegate: nil];
        [w->win setContentView: nil];
        [w->win close];
        [w->win release];
        [w->view release];
        [w->defcur release];
        return 1;
}

int oswSwapBuffers(osw * w) {
        glFlush();
        [wcontext(w)->ctx flushBuffer];
        return 1;
}

int oswShow(osw * w) {
        [w->win makeKeyAndOrderFront: w->view];
        return 1;
}

int oswHide(osw * w) {
        [w->win orderOut: nil];
        return 1;
}

void oswPollEvent(osw * w) {
}

int oswSetSwapInterval(osw * w, int i) {
        GLint param = i;
        [wcontext(w)->ctx setValues:&param forParameter:NSOpenGLCPSwapInterval];
        return 1;
}

int oswClearCurrent(osw * w) {
        glFlush();
        [NSOpenGLContext clearCurrentContext];
        return 1;
}

int oswMakeCurrent(osw * w, osc * c) {
        [c->ctx setView: [w->win contentView]];
        [c->ctx makeCurrentContext];
        return 1;
}

int oswMaximize(osw * w) {
        NSSize scr = [[NSScreen mainScreen] frame].size;
        return oswGeometry(w, 0, 0, scr.width, scr.height);
}

int oswGeometry(osw * w, int x, int y, unsigned width, unsigned height) {
        NSRect r = NSMakeRect(x, reversed(y + height - 1), width, height);
        NSRect fr = [w->win frameRectForContentRect: r];
        [w->win setFrame: fr display: NO];
        return 1;
}

void oswPointer(osw * w) {
        [w->view establishCursor];
}

unsigned oswOrder(osw ** order) {
        NSArray * wins;
        wins = [NSApp orderedWindows];
        unsigned n = [wins count];
        unsigned i;
        for (i = 0; i < n; i++)
                order[i] = ((Window*)[wins objectAtIndex: i])->_w;
        return n;
}

int oscInit(osc * c, osg * g, osc * share) {
        int st = 0; // XXX
        NSOpenGLContext *ctx = 0;
        CGDirectDisplayID dpy = kCGDirectMainDisplay;
        NSOpenGLPixelFormatAttribute attr[] = {
                NSOpenGLPFAFullScreen,
                NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(dpy),
                NSOpenGLPFAColorSize, 24,
                NSOpenGLPFADepthSize, 32,
                NSOpenGLPFAAlphaSize, 8,
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFAAccelerated,
                NSOpenGLPFANoRecovery,
                0, 0, 0, 0, 0, 0, 0, 0,
                };
        NSOpenGLPixelFormat * fmt;
        int last = sizeof(attr) / sizeof(attr[0]);
        while (!attr[--last])
                ;
        last++;
        if (st) 
                attr[last++] = NSOpenGLPFAStereo;
        fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
        if (fmt)
                ctx = [[NSOpenGLContext alloc] 
                              initWithFormat:fmt  
                              shareContext: share? share->ctx : 0];
        [fmt release];
        c->ctx = ctx;
        return ctx != 0;
}

void * oscContextFor(osc * c) {
        return c->ctx;
}

void * oscCurrentContext() {
        return [NSOpenGLContext currentContext];
}

int oscTerm(osc * c) {
        [c->ctx release];
        return 1;
}

int ospInit(osp * p, const void * rgba, unsigned hotx, unsigned hoty) {
        NSBitmapImageRep * b;
        NSImage * img;
        uint8_t * planes[1];
        memcpy(p->rgba, rgba, sizeof(p->rgba));
        planes[0] = p->rgba;
        b = [[NSBitmapImageRep alloc]
                             initWithBitmapDataPlanes: planes
                                           pixelsWide: CURSOR_WIDTH
                                           pixelsHigh: CURSOR_HEIGHT
                                        bitsPerSample: 8
                                      samplesPerPixel: 4
                                             hasAlpha: YES
                                             isPlanar: NO
                                       colorSpaceName: NSDeviceRGBColorSpace
                                          bytesPerRow: CURSOR_WIDTH*4
                                         bitsPerPixel: 32];
        img = [[NSImage alloc] initWithSize: NSMakeSize(CURSOR_WIDTH, 
                                                        CURSOR_HEIGHT)];
        [img addRepresentation: b];
        [b release];
        p->cur = [[NSCursor alloc] initWithImage: img 
                                         hotSpot: NSMakePoint(hotx, hoty)];
        [img release];
        return p->cur != 0;
}

int ospTerm(osp * p) {
        [p->cur release];
        return 1;
}

void osgTick(osg * g) {
}

int osgInit(osg * g, const char * name) {
        return 1;
}

int osgTerm(osg * g) {
        return 1;
}

int main(int argc, char ** argv) {
        int ret;
        NSAutoreleasePool * pool;
        NSApplication * NSApp;
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        SetFrontProcess(&psn);
        pool = [[NSAutoreleasePool alloc] init];
        NSApp = [NSApplication sharedApplication];
        [NSApp setDelegate: [[AppDelegate alloc] init]];
        [NSApp finishLaunching];
        [NSApp activateIgnoringOtherApps: YES];
        progrun(argc, argv);
        [NSApp run];
        ret = progterm();
        [NSApp release];
        [pool release];
        return ret;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/

