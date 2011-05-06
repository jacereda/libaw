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
#include <stdlib.h>
#include <string.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/keysym.h>

#include "aw.h"
#include "awos.h"

#define EVMASK  KeyPressMask | KeyReleaseMask | \
        ButtonPressMask | ButtonReleaseMask |   \
        PointerMotionMask | StructureNotifyMask

#define sync() XSync(wgroup(w)->dpy, False)

static XVisualInfo * chooseVisual(Display * dpy, int screen) {
        int att[64];
        int * p = att;
        *p++ = GLX_RGBA;
        *p++ = GLX_DOUBLEBUFFER;
        *p++ = GLX_RED_SIZE; *p++ = 1;
        *p++ = GLX_GREEN_SIZE; *p++ = 1;
        *p++ = GLX_BLUE_SIZE; *p++ = 1;
        *p++ = GLX_DEPTH_SIZE; *p++ = 1;
        *p++ = None;
        return glXChooseVisual(dpy, screen, att);
}

static void fillWA(unsigned long * swam, XSetWindowAttributes * swa) {
        *swam = 0;
        swa->event_mask = EVMASK; *swam |= CWEventMask;
}

static Window createWin(osg * g, int x, int y, int width, int height) {
        XSetWindowAttributes swa; 
        unsigned long swamask;
        XSizeHints hints;
        Window w;
        fillWA(&swamask, &swa);
        w = XCreateWindow(g->dpy, XRootWindow(g->dpy, g->screen),
                          x, y, width, height, 
                          0, CopyFromParent,
                          InputOutput, 
                          CopyFromParent,
                          swamask, &swa);
        XSelectInput(g->dpy, w, EVMASK);
        XSetWMProtocols(g->dpy, w, &g->del, 1);
        hints.flags = USSize | USPosition;
        hints.x = x; hints.y = y;
        hints.width = width; hints.height = height;
        XSetWMNormalHints(g->dpy, w, &hints);
        return w;
}

static void findBorderSize(osg * g) {
        XEvent e;
        Window w = createWin(g, -100, -100, 1, 1);
        XMapWindow(g->dpy, w);
        while (g->bh < 0) {
                XCheckWindowEvent(g->dpy, w, StructureNotifyMask, &e);
                if (e.type == ConfigureNotify 
                    && !e.xconfigure.override_redirect) {
                        g->bw = e.xconfigure.x;
                        g->bh = e.xconfigure.y;
                }
        }
        XUnmapWindow(g->dpy, w);
        XDestroyWindow(g->dpy, w);
}

int osgInit(osg * g, const char * name) {
        int hasExtensions = 0;
        g->dpy = XOpenDisplay(0);
        if (g->dpy) {
                g->screen = XDefaultScreen(g->dpy);
                g->del = XInternAtom(g->dpy, "WM_DELETE_WINDOW", False);
                findBorderSize(g);
                hasExtensions = 0 != glXQueryExtension(g->dpy, 0, 0);
        }
        if (hasExtensions)
                g->swapInterval = (int(*)(int))glXGetProcAddress(
                        (GLubyte*)"glXSwapIntervalSGI");
        if (!g->swapInterval)
                debug("no glXSwapIntervalSGI()");
        if (g->dpy)
                g->xim = XOpenIM (g->dpy, 0, 0, 0);
        return hasExtensions && g->xim;
}

int osgTerm(osg * g) {
        int ret = 0 == XCloseDisplay(g->dpy);
        return ret;
}

int oswSetTitle(osw * w, const char * t) {
        return XSetStandardProperties(wgroup(w)->dpy, w->win, t, t, None,
                                      (char**)0, 0, 0);
}

static int openwin(osw * w, osg * g, int x, int y, int width, int height) {
        Window win = createWin(g, x, y, width, height);
        XIC xic = 0;
        if (win)
                xic = XCreateIC(
                        g->xim,
                        XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                        XNClientWindow, win,
                        XNFocusWindow, win,
                        NULL);
        w->win = win;
        w->xic = xic;
        return w->win && w->xic;
}

int oswInit(osw * w, osg * g, int x, int y, 
            int width, int height, int bl) {
        int ok;
        if (!bl) {
                x -= g->bw;
                y -= g->bh;
        }
        ok = openwin(w, g, x, y, width, height);
        if (ok && bl) {
                long flags[5] = {0};
                Atom hatom = XInternAtom(g->dpy, "_MOTIF_WM_HINTS", 1);
                flags[0] = 2; //mwm.flags = MWM_HINTS_DECORATIONS;
                flags[2] = 0;
                XChangeProperty(g->dpy, w->win, hatom, hatom, 
                                32, PropModeReplace,
                                (unsigned char*)flags, sizeof(flags) / 4);
        }
        if (g->dpy)
                XSync(g->dpy, False);
        return ok;
}

int oswTerm(osw * w) {
        XDestroyIC(w->xic);
        XDestroyWindow(wgroup(w)->dpy, w->win);
        return 1;
}

int oswSwapBuffers(osw * w) {
        glXSwapBuffers(wgroup(w)->dpy, w->win);
        return 1;
}

int oswMakeCurrent(osw * w, osc * c) {
        return glXMakeCurrent(wgroup(w)->dpy, w->win, c->ctx);
}

int oswClearCurrent(osw * w) {
        return glXMakeCurrent(wgroup(w)->dpy, 0, 0);
}

int oswShow(osw * w) {
        int ret = 0; 
        ret |= XMapWindow(wgroup(w)->dpy, w->win);
        ret |= sync();
        return ret;
}

int oswHide(osw * w) {
        int ret = 0;
        ret |= XUnmapWindow(wgroup(w)->dpy, w->win);
        ret |= sync();
        return ret;
}


static int mapButton(int button) {
        int which;
        switch (button) {
        case Button1: which = AW_KEY_MOUSELEFT; break;
        case Button2: which = AW_KEY_MOUSEMIDDLE; break;
        case Button3: which = AW_KEY_MOUSERIGHT; break;
        case Button4: which = AW_KEY_MOUSEWHEELUP; break;
        case Button5: which = AW_KEY_MOUSEWHEELDOWN; break;
        default: which = AW_KEY_NONE;
        }
        return which;
}

static void configure(osw * w, int x, int y, int width, int height) {
        if (width != w->lastw || height != w->lasth)
                got(w, AW_EVENT_RESIZE, width, height);
        w->lastw = width; w->lasth = height;
        if (x != w->lastx || y != w->lasty)
                got(w, AW_EVENT_POSITION, x, y);
        w->lastx = x; w->lasty = y;
}

static int isAutoRepeat(osw * w, XEvent * e) {
        XEvent next;
        XCheckTypedWindowEvent(wgroup(w)->dpy, w->win, KeyPress, &next);
        XPutBackEvent(wgroup(w)->dpy, &next);
        return next.xkey.keycode == e->xkey.keycode
                && next.xkey.time - e->xkey.time < 2;
}

static void handle(osw * w, XEvent * e) {
        extern unsigned mapkeycode(unsigned);
        switch(e->type) {
        case ClientMessage:
                got(w, AW_EVENT_CLOSE, 0, 0);
                break;
        case ConfigureNotify:
        {
                int x, y;
                Window child;
                XTranslateCoordinates(wgroup(w)->dpy, w->win, XRootWindow(wgroup(w)->dpy, 0), 
                                      0, 0, &x, &y, &child);
                configure(w, x, y, //e->xconfigure.x, e->xconfigure.y,
                          e->xconfigure.width, e->xconfigure.height);
        }
                break;
        case ButtonPress:
                got(w, AW_EVENT_DOWN, mapButton(e->xbutton.button), 0);
                break;
        case ButtonRelease:
                got(w, AW_EVENT_UP, mapButton(e->xbutton.button), 0);
                break;
        case MotionNotify:
                got(w, AW_EVENT_MOTION, e->xmotion.x, e->xmotion.y);
                break;
        case KeyPress:
        {
                wchar_t buf[64];
                KeySym ks;
                Status st;
                int i;
                int n;
                got(w, AW_EVENT_DOWN, mapkeycode(e->xkey.keycode), 0);
                n = XwcLookupString(w->xic,
                                    &e->xkey, 
                                    buf, 
                                    sizeof(buf) / sizeof(wchar_t),
                                    &ks, 
                                    &st);
                if (st == XLookupChars || st == XLookupBoth) 
                        for (i = 0; i < n; i++)
                                got(w, AW_EVENT_UNICODE, buf[i], 0);
        }
        break;
        case KeyRelease:
                if (!isAutoRepeat(w, e)) 
                        got(w, AW_EVENT_UP, mapkeycode(e->xkey.keycode), 0);

        default:
                got(w, AW_EVENT_UNKNOWN, 0, 0);
                break;
        }
}

static int pollEvent(osw * w, XEvent * e) {
        return XCheckWindowEvent(wgroup(w)->dpy, w->win, EVMASK, e)
                || XCheckTypedWindowEvent(wgroup(w)->dpy, w->win, ClientMessage, e);
}

void oswPollEvent(osw * w) {
        XEvent e;
        sync();
        if (pollEvent(w, &e))
                handle(w, &e);
}

int oswSetSwapInterval(osw * w, int interval) {
#if 1
        return !wgroup(w)->swapInterval 
                || 0 == wgroup(w)->swapInterval(interval);
#else
        if (group(w)->swapInterval)
                group(w)->swapInterval(interval);
        return 1;
#endif
}

int oswMaximize(osw * w) {
        osg * g = wgroup(w);
        unsigned width = DisplayWidth(g->dpy, g->screen);
        unsigned height = DisplayHeight(g->dpy, g->screen);
        return oswGeometry(w, 0, 0, width, height);
}

int oswGeometry(osw * w, int x, int y, unsigned width, unsigned height) {
        int ret = 0;
        ret |= XMoveResizeWindow(wgroup(w)->dpy, w->win, x - wgroup(w)->bw, y - wgroup(w)->bh, 
                                 width, height);
        ret |= sync();
        return ret;
}

int oscInit(osc * c, osg * g, osc * share) {
        XVisualInfo * vinfo = chooseVisual(g->dpy, g->screen);
        GLXContext ctx = 0;
        if (vinfo) {
                ctx = glXCreateContext(g->dpy, vinfo, 
                                       share? share->ctx : 0, True);
                XFree(vinfo);
        }
        c->ctx = ctx;
        return c->ctx != 0;
}

int oscTerm(osc * c) {
        glXDestroyContext(cgroup(c)->dpy, c->ctx);
        return 1;
}

int ospInit(osp * p, const void * rgba, unsigned hotx, unsigned hoty) {
        return 1;
}

int ospTerm(osp * p) {
        return 1;
}

unsigned oswOrder(osw ** w) {
        return 0;
}

void oswPointer(osw * w) {

}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
