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
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/keysym.h>

#include "aw.h"
#include "awos.h"

#define EVMASK  KeyPressMask | KeyReleaseMask | \
        ButtonPressMask | ButtonReleaseMask |   \
        PointerMotionMask | StructureNotifyMask
static Display * g_dpy;
static Atom g_del;
static int g_screen;
static XIM g_xim = 0;

static int (*g_SwapInterval)(int);

struct _aw {
        awHeader hdr;
        Window win;
        int lastw, lasth;
        XIC xic;
};

struct _ac {
        acHeader hdr;
        GLXContext ctx;
};

static int sync() {
        return XSync(g_dpy, False);
}

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

static Window createWin(int x, int y, int width, int height) {
        XSetWindowAttributes swa; 
        unsigned long swamask;
        XSizeHints hints;
        Window w;
        fillWA(&swamask, &swa);
        w = XCreateWindow(g_dpy, XRootWindow(g_dpy, g_screen),
                          x, y, width, height, 
                          0, CopyFromParent,
                          InputOutput, 
                          CopyFromParent,
                          swamask, &swa);
        XSelectInput(g_dpy, w, EVMASK);
        XSetWMProtocols(g_dpy, w, &g_del, 1);
        hints.flags = USSize | USPosition;
        hints.x = x; hints.y = y;
        hints.width = width; hints.height = height;
        XSetWMNormalHints(g_dpy, w, &hints);
        return w;
}

static int g_bw = -1;
static int g_bh = -1;

static void findBorderSize() {
        XEvent e;
        Window w = createWin(-100, -100, 1, 1);
        XMapWindow(g_dpy, w);
        while (g_bh < 0) {
                XCheckWindowEvent(g_dpy, w, StructureNotifyMask, &e);
                if (e.type == ConfigureNotify 
                    && !e.xconfigure.override_redirect) {
                        g_bw = e.xconfigure.x;
                        g_bh = e.xconfigure.y;
                }
        }
        XUnmapWindow(g_dpy, w);
        XDestroyWindow(g_dpy, w);
        sync();
}

int awosInit() {
        g_dpy = XOpenDisplay(0);
        int hasExtensions = 0;
        if (g_dpy) {
                g_screen = XDefaultScreen(g_dpy);
                g_del = XInternAtom(g_dpy, "WM_DELETE_WINDOW", False);
                findBorderSize();
                hasExtensions = 0 != glXQueryExtension(g_dpy, 0, 0);
        }
        if (hasExtensions)
                g_SwapInterval = (int(*)(int))glXGetProcAddress(
                        (GLubyte*)"glXSwapIntervalSGI");
        if (!g_SwapInterval)
                report("no glXSwapIntervalSGI()");
        if (g_dpy)
                g_xim = XOpenIM (g_dpy, NULL, NULL, NULL);
        return hasExtensions && g_xim;
}

int awosEnd() {
        return 0 == XCloseDisplay(g_dpy);
}

int awosSetTitle(aw * w, const char * t) {
        return XSetStandardProperties(g_dpy, w->win, t, t, None,
                                      (char**)NULL, 0, NULL);
}

static aw * openwin(int x, int y, int width, int height) {
        aw * w = NULL;
        Window win = createWin(x, y, width, height);
        XIC xic = 0;
        if (win)
                xic = XCreateIC(
                        g_xim,
                        XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                        XNClientWindow, win,
                        XNFocusWindow, win,
                        NULL);
        if (xic)
                w = calloc(1, sizeof(*w));
        if (w) {
                w->win = win;
                w->xic = xic;
        }
        return w;
}

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
        aw * w;
        if (fs) {
                width = DisplayWidth(g_dpy, g_screen);
                height = DisplayHeight(g_dpy, g_screen);
        }
        if (!bl) {
                x -= g_bw;
                y -= g_bh;
        }
        w = openwin(x, y, width, height);
        if (bl) {
                long flags[5] = {0};
                Atom hatom = XInternAtom(g_dpy, "_MOTIF_WM_HINTS", 1);
                flags[0] = 2; //mwm.flags = MWM_HINTS_DECORATIONS;
                flags[2] = 0;
                XChangeProperty(g_dpy, w->win, hatom, hatom, 
                                32, PropModeReplace,
                                (unsigned char*)flags, sizeof(flags) / 4);
        }
        if (g_dpy)
                sync();
        return w;
}

int awosClose(aw * w) {
        XDestroyIC(w->xic);
        XDestroyWindow(g_dpy, w->win);
        free(w);
        return 1;
}

int awosSwapBuffers(aw * w) {
        glXSwapBuffers(g_dpy, w->win);
        return 1;
}

int awosMakeCurrent(aw * w, ac * c) {
        return glXMakeCurrent(g_dpy, w->win, c->ctx);
}

int awosClearCurrent(aw * w) {
        return glXMakeCurrent(g_dpy, 0, 0);
}

int awosShow(aw * w) {
        int ret = 0; 
        ret |= XMapWindow(g_dpy, w->win);
        ret |= sync();
        return ret;
}

int awosHide(aw * w) {
        int ret = 0;
        ret |= XUnmapWindow(g_dpy, w->win);
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

static unsigned key2aw(unsigned k) {
        int ret = k;
        switch (k) {
	case XK_A: ret = AW_KEY_A; break;
	case XK_S: ret = AW_KEY_S; break;
	case XK_D: ret = AW_KEY_D; break;
	case XK_F: ret = AW_KEY_F; break;
	case XK_H: ret = AW_KEY_H; break;
	case XK_G: ret = AW_KEY_G; break;
	case XK_Z: ret = AW_KEY_Z; break;
	case XK_X: ret = AW_KEY_X; break;
	case XK_C: ret = AW_KEY_C; break;
	case XK_V: ret = AW_KEY_V; break;
	case XK_B: ret = AW_KEY_B; break;
	case XK_Q: ret = AW_KEY_Q; break;
	case XK_W: ret = AW_KEY_W; break;
	case XK_E: ret = AW_KEY_E; break;
	case XK_R: ret = AW_KEY_R; break;
	case XK_Y: ret = AW_KEY_Y; break;
	case XK_T: ret = AW_KEY_T; break;
	case XK_1: ret = AW_KEY_1; break;
	case XK_2: ret = AW_KEY_2; break;
	case XK_3: ret = AW_KEY_3; break;
	case XK_4: ret = AW_KEY_4; break;
	case XK_6: ret = AW_KEY_6; break;
	case XK_5: ret = AW_KEY_5; break;
	case XK_equal: ret = AW_KEY_EQUAL; break;
	case XK_9: ret = AW_KEY_9; break;
	case XK_7: ret = AW_KEY_7; break;
	case XK_minus: ret = AW_KEY_MINUS; break;
	case XK_8: ret = AW_KEY_8; break;
	case XK_0: ret = AW_KEY_0; break;
	case XK_bracketright: ret = AW_KEY_RIGHTBRACKET; break;
	case XK_O: ret = AW_KEY_O; break;
	case XK_U: ret = AW_KEY_U; break;
	case XK_bracketleft: ret = AW_KEY_LEFTBRACKET; break;
	case XK_I: ret = AW_KEY_I; break;
	case XK_P: ret = AW_KEY_P; break;
	case XK_L: ret = AW_KEY_L; break;
	case XK_J: ret = AW_KEY_J; break;
	case XK_apostrophe: ret = AW_KEY_QUOTE; break;
	case XK_K: ret = AW_KEY_K; break;
	case XK_semicolon: ret = AW_KEY_SEMICOLON; break;
	case XK_backslash: ret = AW_KEY_BACKSLASH; break;
	case XK_comma: ret = AW_KEY_COMMA; break;
	case XK_slash: ret = AW_KEY_SLASH; break;
	case XK_N: ret = AW_KEY_N; break;
	case XK_M: ret = AW_KEY_M; break;
	case XK_period: ret = AW_KEY_PERIOD; break;
	case XK_grave: ret = AW_KEY_GRAVE; break;
	case XK_KP_Decimal: ret = AW_KEY_KEYPADDECIMAL; break;
	case XK_KP_Multiply: ret = AW_KEY_KEYPADMULTIPLY; break;
	case XK_KP_Add: ret = AW_KEY_KEYPADPLUS; break;
	case XK_Clear: ret = AW_KEY_KEYPADCLEAR; break;
	case XK_KP_Divide: ret = AW_KEY_KEYPADDIVIDE; break;
	case XK_KP_Enter: ret = AW_KEY_KEYPADENTER; break;
	case XK_KP_Subtract: ret = AW_KEY_KEYPADMINUS; break;
	case XK_KP_Equal: ret = AW_KEY_KEYPADEQUALS; break;
	case XK_KP_0: ret = AW_KEY_KEYPAD0; break;
	case XK_KP_1: ret = AW_KEY_KEYPAD1; break;
	case XK_KP_2: ret = AW_KEY_KEYPAD2; break;
	case XK_KP_3: ret = AW_KEY_KEYPAD3; break;
	case XK_KP_4: ret = AW_KEY_KEYPAD4; break;
	case XK_KP_5: ret = AW_KEY_KEYPAD5; break;
	case XK_KP_6: ret = AW_KEY_KEYPAD6; break;
	case XK_KP_7: ret = AW_KEY_KEYPAD7; break;
	case XK_KP_8: ret = AW_KEY_KEYPAD8; break;
	case XK_KP_9: ret = AW_KEY_KEYPAD9; break;
	case XK_Return: ret = AW_KEY_RETURN; break;
	case XK_Tab: ret = AW_KEY_TAB; break;
	case XK_KP_Space: ret = AW_KEY_SPACE; break;
	case XK_BackSpace: ret = AW_KEY_DELETE; break;
	case XK_Escape: ret = AW_KEY_ESCAPE; break;
	case XK_Meta_L: ret = AW_KEY_COMMAND; break;
	case XK_Shift_L: ret = AW_KEY_SHIFT; break;
	case XK_Caps_Lock: ret = AW_KEY_CAPSLOCK; break;
	case XK_Alt_L: ret = AW_KEY_OPTION; break;
	case XK_Control_L: ret = AW_KEY_CONTROL; break;
	case XK_Shift_R: ret = AW_KEY_RIGHTSHIFT; break;
	case XK_Alt_R: ret = AW_KEY_RIGHTOPTION; break;
	case XK_Control_R: ret = AW_KEY_RIGHTCONTROL; break;
//	case XK_function: ret = AW_KEY_FUNCTION; break;
	case XK_F17: ret = AW_KEY_F17; break;
//	case XK_VolumeUp: ret = AW_KEY_VOLUMEUP; break;
//	case XK_VolumeDown: ret = AW_KEY_VOLUMEDOWN; break;
//	case XK_Mute: ret = AW_KEY_MUTE; break;
	case XK_F18: ret = AW_KEY_F18; break;
	case XK_F19: ret = AW_KEY_F19; break;
	case XK_F20: ret = AW_KEY_F20; break;
	case XK_F5: ret = AW_KEY_F5; break;
	case XK_F6: ret = AW_KEY_F6; break;
	case XK_F7: ret = AW_KEY_F7; break;
	case XK_F3: ret = AW_KEY_F3; break;
	case XK_F8: ret = AW_KEY_F8; break;
	case XK_F9: ret = AW_KEY_F9; break;
	case XK_F11: ret = AW_KEY_F11; break;
	case XK_F13: ret = AW_KEY_F13; break;
	case XK_F16: ret = AW_KEY_F16; break;
	case XK_F14: ret = AW_KEY_F14; break;
	case XK_F10: ret = AW_KEY_F10; break;
	case XK_F12: ret = AW_KEY_F12; break;
	case XK_F15: ret = AW_KEY_F15; break;
	case XK_Help: ret = AW_KEY_HELP; break;
	case XK_Home: ret = AW_KEY_HOME; break;
	case XK_Page_Up: ret = AW_KEY_PAGEUP; break;
	case XK_Delete: ret = AW_KEY_FORWARDDELETE; break;
	case XK_F4: ret = AW_KEY_F4; break;
	case XK_End: ret = AW_KEY_END; break;
	case XK_F2: ret = AW_KEY_F2; break;
	case XK_Page_Down: ret = AW_KEY_PAGEDOWN; break;
	case XK_F1: ret = AW_KEY_F1; break;
	case XK_Left: ret = AW_KEY_LEFTARROW; break;
	case XK_Right: ret = AW_KEY_RIGHTARROW; break;
	case XK_Down: ret = AW_KEY_DOWNARROW; break;
	case XK_Up: ret = AW_KEY_UPARROW; break;
        default: ret = k;
        }
        return ret;
}

static int mapKey(KeyCode keycode) {
        return key2aw(XKeycodeToKeysym(g_dpy, keycode, 0));
}

static void configure(aw * w, int x, int y, int width, int height) {
        if (width != w->lastw || height != w->lasth)
                got(w, AW_EVENT_RESIZE, width, height);
        w->lastw = width; w->lasth = height;
}

static int isAutoRepeat(XEvent * e, Window win) {
        XEvent next;
        XCheckTypedWindowEvent(g_dpy, win, KeyPress, &next);
        XPutBackEvent(g_dpy, &next);
        return next.xkey.keycode == e->xkey.keycode
                && next.xkey.time - e->xkey.time < 2;
}

static void handle(aw * w, XEvent * e) {
        switch(e->type) {
        case ClientMessage:
                got(w, AW_EVENT_CLOSE, 0, 0);
                break;
        case ConfigureNotify:
                configure(w, e->xconfigure.x, e->xconfigure.y,
                          e->xconfigure.width, e->xconfigure.height);
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
#define MAXCHARS 64
                wchar_t buf[MAXCHARS];
                KeySym ks;
                Status st;
                int i;
                int n = XwcLookupString(w->xic,
                                        &e->xkey, 
                                        buf, 
                                        MAXCHARS,
                                        &ks, 
                                        &st);
                if (st == XLookupKeySym || st == XLookupBoth) 
                        got(w, AW_EVENT_DOWN, 
                            key2aw(ks), 0);
                if (st == XLookupChars || st == XLookupBoth) 
                        for (i = 0; i < n; i++)
                                got(w, AW_EVENT_UNICODE, buf[i], 0);
        }
        break;
        case KeyRelease:
                if (!isAutoRepeat(e, w->win))
                        got(w, AW_EVENT_UP, mapKey(e->xkey.keycode), 0);
                break;
        default:
                got(w, AW_EVENT_UNKNOWN, 0, 0);
                break;
        }
}

static int pollEvent(aw * w, XEvent * e) {
        return XCheckWindowEvent(g_dpy, w->win, EVMASK, e)
                || XCheckTypedWindowEvent(g_dpy, w->win, ClientMessage, e);
}

void awosPollEvent(aw * w) {
        XEvent e;
        sync();
        if (pollEvent(w, &e))
                handle(w, &e);
}

int awosSetSwapInterval(aw * w, int interval) {
        return !g_SwapInterval || 0 == g_SwapInterval(interval);
}

ac * acosNew(ac * share) {
        XVisualInfo * vinfo = chooseVisual(g_dpy, g_screen);
        GLXContext ctx;
        ac * c = 0;
        if (vinfo) {
                ctx = glXCreateContext(g_dpy, vinfo, 
                                       share? share->ctx : 0, True);
                XFree(vinfo);
        }
        if (ctx)
                c = calloc(1, sizeof(*c));
        if (c)
                c->ctx = ctx;
        return c;
}

int acosDel(ac * c) {
        glXDestroyContext(g_dpy, c->ctx);
        free(c);
        return 1;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
