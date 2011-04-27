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
#include "aw.h"
#include "awos.h"
#include <assert.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSString.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSNotification.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSScreen.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSEvent.h>
#include <AppKit/NSOpenGL.h>
#include <AppKit/NSTextView.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

@interface Window : NSWindow {
@public
        aw * _w;
        ac * _c;
}
@end

@interface View : NSTextView<NSWindowDelegate> {
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

- (void)setConstrainedFrameSize:(NSSize)desiredSize{}


- (BOOL) acceptsFirstResponder {
        return YES;
}

- (void) windowDidResize:(NSNotification *)n {
        NSSize sz = [_w->view frame].size;
        if (_w->hdr.ctx) [_w->hdr.ctx->ctx update];
        got(_w, AW_EVENT_RESIZE, (int)sz.width, (int)sz.height);
}

- (BOOL) windowShouldClose: (id)s {
        got(_w, AW_EVENT_CLOSE, 0, 0);
        return NO;
}

extern awkey mapkey(unsigned);

- (void) handleKeyEvent: (NSEvent *)ev mode: (int) mode {
        got(_w, mode, mapkey([ev keyCode]), 0);
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

- (awkey) keyFor: (unsigned)mask {
        awkey ret = 0;
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
        [self handleMouseEvent: ev mode: AW_EVENT_DOWN];
}

- (void) mouseUp: (NSEvent*)ev {
        [self handleMouseEvent: ev mode: AW_EVENT_UP];
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
        awkey k = [ev deltaY] > 0? 
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

- (BOOL) isOpaque {
        return YES;
}
@end

@implementation Window
- (BOOL) canBecomeKeyWindow
{
        return YES;
}
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
        //[g_apppool release];
        return 1;
}

int awosSetTitle(aw * w, const char * t) {
        NSString * s = [[NSString alloc] initWithUTF8String: t];
        [w->win setTitle: s];
        [s release];
        return 1;
}

static aw * openwin(int x, int y, int width, int height, unsigned style) {
        NSRect rect;
        Window * win;
        NSRect frm;
        View * view;
        aw * w = 0;
        NSSize scr = [[NSScreen mainScreen] frame].size;
        rect = NSMakeRect(x, scr.height - y - height, width, height);
        win = [[Window alloc] initWithContentRect: rect 
                              styleMask: style
                              backing: NSBackingStoreBuffered
                              defer:NO];
        frm = [Window contentRectForFrameRect: [win frame] styleMask: style];
        view = [[View alloc] initWithFrame: [[win contentView] frame]];
        [view setAutoresizesSubviews:YES];

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

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
        unsigned style = 0;
        aw * w;
        if (fs) {
                NSSize scr = [[NSScreen mainScreen] frame].size;
                width = scr.width;
                height = scr.height;
        }
        if (!bl) {
                style += 0
                        | NSTitledWindowMask
                        | NSClosableWindowMask
                        | NSMiniaturizableWindowMask 
                        | NSResizableWindowMask
                        ;
        }
        w = openwin(x, y, width, height, style);
        if (bl) 
                [w->win setLevel: NSPopUpMenuWindowLevel];
        return w;
}

static NSEvent * nextEvent(Window * win) {
        NSEvent * e = [win nextEventMatchingMask: NSAnyEventMask 
                           untilDate: [NSDate distantPast] 
                           inMode: NSDefaultRunLoopMode 
                           dequeue: YES];
        [NSApp sendEvent: e];
        [NSApp updateWindows];
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
        [w->hdr.ctx->ctx flushBuffer];
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

int awosSetSwapInterval(aw * w, int i) {
        GLint param = i;
        [w->hdr.ctx->ctx setValues:&param forParameter:NSOpenGLCPSwapInterval];
        return 1;
}

int awosClearCurrent(aw * w) {
        [w->hdr.ctx->ctx clearDrawable];
        [NSOpenGLContext clearCurrentContext];
        return 1;
}

int awosMakeCurrent(aw * w, ac * c) {
        [c->ctx setView: [w->win contentView]];
        [c->ctx makeCurrentContext];
        return 1;
}

ac * acosNew(ac * share) {
        ac * c = 0;
        NSOpenGLContext *ctx = 0;
        CGDirectDisplayID dpy = kCGDirectMainDisplay;
        NSOpenGLPixelFormatAttribute attr[] = {
                NSOpenGLPFAFullScreen,
                NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(dpy),
                NSOpenGLPFAColorSize, 24,
                NSOpenGLPFAAlphaSize, 8,
                NSOpenGLPFADepthSize, 16,
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFAAccelerated,
                NSOpenGLPFANoRecovery,
                0};
        NSOpenGLPixelFormat * fmt;
        fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
        if (fmt)
                ctx = [[NSOpenGLContext alloc] 
                              initWithFormat:fmt 
                              shareContext: share? share->ctx : 0];
        [fmt release];
        if (ctx)
                c = calloc(1, sizeof(*c));
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
   indent-tabs-mode: nil **
   End: **
*/
