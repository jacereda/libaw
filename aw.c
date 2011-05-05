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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "sysgl.h"
#include "aw.h"
#include "awos.h"
#include "bit.h"

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

static int gcheck(const ag * g) {
        if (!g)
                report("Null group handle");
        return g != 0;
}

static int ccheck(const ac * c) {
        if (!c)
                report("Null context handle");
        return c != 0;
}

static int wcheck(const aw * w) {
        if (!w)
                report("Null window handle");
        return w != 0;
}

static int pcheck(const ap * p) {
        if (!p)
                report("Null pointer handle");
        return p != 0;
}

ag * agNew(const char * appname) {
        ag * g = calloc(1, sizeof(*g));
        int ok;
        ok = osgInit(&g->osg, appname);
        if (!ok) {
                report("creating group %s", appname);
                free(g);
                g = 0;
        }
        return g;
}

void agDel(ag * g) {
        if (gcheck(g)) {
                if (!osgTerm(&g->osg))
                        report("deleting group");
                free(g);
        }
}

void awShow(aw * w) {
        if (wcheck(w) && !oswShow(&w->osw))
                report("Unable to show window");
        else
                w->shown = 1;
}

void awHide(aw * w) {
        if (wcheck(w) && !oswHide(&w->osw))
                report("Unable to hide window");
        else
                w->shown = 0;
}

void awSetTitle(aw * w, const char * t) {
        if (wcheck(w) && !w->fullscreen && !oswSetTitle(&w->osw, t))
                report("Unable to set window title");
}

static int wopen(ag * g, aw * w, int fs, int bl) {
        int ok;
        memset(&w->osw, 0, sizeof(w->osw));
        w->g = g;
        ok = oswInit(&w->osw, &g->osg, 0, 0, 10, 10, fs, bl);
        if (!ok)
                report("Unable to open window");
        w->fullscreen = fs;
        w->last = w->ev;
        w->last->type = AW_EVENT_NONE;
        w->g = g;
        return ok;
}

static void wclose(aw * w) {
        awPointer(w, 0);
        oswClearCurrent(&w->osw);
        oswHide(&w->osw);
        while (awNextEvent(w))
                ;
        if (!oswTerm(&w->osw))
                report("Unable to close window");
}

static int wreopen(aw * w, int fs, int bl) {
        int ok;
        wclose(w);
        ok = wopen(w->g, w, fs, bl);
        if (w->shown)
                oswShow(&w->osw);
        if (w->ctx)
                oswMakeCurrent(&w->osw, &w->ctx->osc);
        return ok;
}

aw * awNew(ag * g) { 
        aw * w = calloc(1, sizeof(*w));
        int ok = wopen(g, w, 0, 0);
        if (!ok) {
                free(w);
                w = 0;
        }
        return w;
}

void awDel(aw * w) {  
        if (wcheck(w)) {
                if (w->pointer) 
                        report("Closing window with cursor established");
                if (w->ctx) 
                        report("Closing window with active context");
                wclose(w);
        }
}

void awPlain(aw * w) {
        wreopen(w, 0, 0);
}

void awBorderless(aw * w) {
        wreopen(w, 0, 1);
}

void awFullscreen(aw * w) {
        wreopen(w, 1, 0);
}

void awMaximize(aw * w) {
        wreopen(w, 1, 1);
}

static void setInterval(aw * w) {
        if (w->interval && !oswSetSwapInterval(&w->osw, w->interval)) 
                report("Unable to set swap interval");
}

void awSwapBuffers(aw * w) {
        if (wcheck(w)) {
                setInterval(w);
                glFlush();
                if (!w->ctx)
                        report("awSwapBuffers called without context");
                else if (!oswSwapBuffers(&w->osw))
                        report("awSwapBuffers failed");
                memcpy(w->ppressed, w->pressed, sizeof(w->ppressed));
        }
}

void awMakeCurrent(aw * w, ac * c) {
        if (wcheck(w)) {
                if (w->ctx)
                        glFlush();
                if (w->ctx && w->ctx != c) 
                        oswClearCurrent(&w->osw);
                if (c && !oswMakeCurrent(&w->osw, &c->osc))
                        report("Unable to establish context");
                w->ctx = c;
        }
}

static unsigned char * bitarrayFor(unsigned char * p, awkey * k) {
        int good = *k >= AW_KEY_NONE && *k < AW_KEY_MAX;
        assert(good);
        *k -= AW_KEY_NONE;
        return good? p : 0;
}

static const unsigned char * bitarrayForC(const unsigned char * p, awkey * k) {
        return (const unsigned char *)bitarrayFor((unsigned char *)p, k);
}

static int pressed(const aw * w, awkey k) {
        const unsigned char * ba = bitarrayForC(w->pressed, &k);
        return ba && bittest(ba, k);
}

static void press(aw * w, awkey k) {
        unsigned char * ba = bitarrayFor(w->pressed, &k);
        if (ba)
                bitset(ba, k);
}

static void release(aw * w, awkey k) {
        unsigned char * ba = bitarrayFor(w->pressed, &k);
        if (ba)
                bitclear(ba, k);
}

const ae * awNextEvent(aw * w) {
        const ae * e = 0;
        if (!wcheck(w))
                return 0;
        assert(w->last);
        if (w->last->type == AW_EVENT_DROP) {
                ae * last = w->last;
                assert(last->p[0]);
                free((void*)last->p[0]);
                last->p[0] = 0;
                last->type = AW_EVENT_NONE;
        }
        oswPollEvent(&w->osw);
        if (w->head != w->tail) {
                e = w->ev + w->tail;
                w->tail++;
                w->tail %= MAX_EVENTS;
        }
        if (e) switch (aeType(e)) {   
                case AW_EVENT_RESIZE:
                        w->width = aeWidth(e);
                        w->height = aeHeight(e);
                        break;
                case AW_EVENT_MOTION:
                        w->mx = aeX(e);
                        w->my = aeY(e);
                        break;
                case AW_EVENT_DOWN:
                        press(w, aeWhich(e));
                        break;
                case AW_EVENT_UP:
                        release(w, aeWhich(e));
                        break;
                default: break;
                }
        if (e)
                w->last = (ae*)e;
        return e;
}

unsigned awWidth(const aw * w) {
        return w->width;
}

unsigned awHeight(const aw * w) {
        return w->height;
}

int awMouseX(const aw * w) {
        return w->mx;
}

int awMouseY(const aw * w) {
        return w->my;
}

int awPressed(const aw * w, awkey k) {
        return pressed(w, k);
}

int awReleased(const aw * w, awkey k) {
        awkey pk = k;
        const unsigned char * ba = bitarrayForC(w->pressed, &k);
        const unsigned char * pba = bitarrayForC(w->ppressed, &pk);
        return ba && !bittest(ba, k) && bittest(pba, k);
}

void got(osw * osw, int type, intptr_t p1, intptr_t p2) {
        aw * w = (aw*)osw;
        ae * e = w->ev + w->head;
        w->head++;
        w->head %= MAX_EVENTS;
        e->type = type;
        e->p[0] = p1;
        e->p[1] = p2;
}

ac * acNew(ag * g, ac * share) {
        ac * c = calloc(1, sizeof(*c));
        int ok;
        c->g = g;
        ok = oscInit(&c->osc, &g->osg, &share->osc);
        if (!ok) {
                report("unable to create context sharing with %p", share);
                free(c);
                c = 0;
        }
        return c;
}

ac * acNewStereo(ag * g, ac * share) {
        return 0; // XXX
}

void acDel(ac * c) {
        if (ccheck(c)) {
                if (!oscTerm(&c->osc))
                        report("unable to delete context %p", c);
                free(c);
        }
}

void awSetInterval(aw * w, int interval) {
        w->interval = interval;
}

void awSetUserData(aw * w, void * user) {
        w->user = user;
}

void * awUserData(const aw * w) {
        return w->user;
}

void awGeometry(aw * w, int x, int y, unsigned width, unsigned height) {
        if (wcheck(w) && !oswGeometry(&w->osw, x, y, width, height))
                report("unable to establish geometry");
}

void awPointer(aw * w, ap * p) {
        if (w->pointer)
                w->pointer->refs--;
        w->pointer = p;
        if (w->pointer)
                w->pointer->refs++;
        oswPointer(&w->osw);
}

ap * apNew(const void * rgba, unsigned hotx, unsigned hoty) {
        ap * p = calloc(1, sizeof(*p));
        int ok;
        ok = ospInit(&p->osp, rgba, hotx, hoty);
        if (!ok) {
                report("unable to create pointer");
                free(p);
                p = 0;
        }
        return p;
}

void apDel(ap * p) {
        if (p->refs)
                report("destroying referenced pointer");
        else if (pcheck(p)) {
                if (!ospTerm(&p->osp))
                        report("unable to destroy pointer");
                free(p);
        }
}

unsigned awOrder(const aw * w) {
        aw * zorder[MAX_WINDOWS];
        unsigned n = oswOrder((osw**)zorder);
        unsigned i = 0;
        assert(n < MAX_WINDOWS);
#if !defined NDEBUG
        for (i = 0; i < n; i++)
                assert(zorder[i]);
#endif
        for (i = 0; i < n; i++)
                if (w == zorder[i])
                        break;
        return i;
}

int aeType(const ae * e) {
        return e->type;
}

int aeWidth(const ae * e) {
        return e->p[0];
}

int aeHeight(const ae * e) {
        return e->p[1];
}

awkey aeWhich(const ae * e) {
        return e->p[0];
}

int aeX(const ae * e) {
        return e->p[0];
}

int aeY(const ae * e) {
        return e->p[1];
}

const char * aePath(const ae * e) {
        return (const char *)e->p[0];
}

const char * aeKeyName(const ae * e) {
	const char * ret = 0;
	static char buf[32] = {0};
        awkey k = aeWhich(e);
	switch (k) {
	case AW_KEY_NONE: ret = "NONE"; break;
	case AW_KEY_MOUSEWHEELUP: ret = "MOUSEWHEELUP"; break;
	case AW_KEY_MOUSEWHEELDOWN: ret = "MOUSEWHEELDOWN"; break;
	case AW_KEY_MOUSELEFT: ret = "MOUSELEFT"; break;
	case AW_KEY_MOUSEMIDDLE: ret = "MOUSEMIDDLE"; break;
	case AW_KEY_MOUSERIGHT: ret = "MOUSERIGHT"; break;

	case AW_KEY_A: ret = "AW_KEY_A"; break;
	case AW_KEY_S: ret = "AW_KEY_S"; break;
	case AW_KEY_D: ret = "AW_KEY_D"; break;
	case AW_KEY_F: ret = "AW_KEY_F"; break;
	case AW_KEY_H: ret = "AW_KEY_H"; break;
	case AW_KEY_G: ret = "AW_KEY_G"; break;
	case AW_KEY_Z: ret = "AW_KEY_Z"; break;
	case AW_KEY_X: ret = "AW_KEY_X"; break;
	case AW_KEY_C: ret = "AW_KEY_C"; break;
	case AW_KEY_V: ret = "AW_KEY_V"; break;
	case AW_KEY_B: ret = "AW_KEY_B"; break;
	case AW_KEY_Q: ret = "AW_KEY_Q"; break;
	case AW_KEY_W: ret = "AW_KEY_W"; break;
	case AW_KEY_E: ret = "AW_KEY_E"; break;
	case AW_KEY_R: ret = "AW_KEY_R"; break;
	case AW_KEY_Y: ret = "AW_KEY_Y"; break;
	case AW_KEY_T: ret = "AW_KEY_T"; break;
	case AW_KEY_1: ret = "AW_KEY_1"; break;
	case AW_KEY_2: ret = "AW_KEY_2"; break;
	case AW_KEY_3: ret = "AW_KEY_3"; break;
	case AW_KEY_4: ret = "AW_KEY_4"; break;
	case AW_KEY_6: ret = "AW_KEY_6"; break;
	case AW_KEY_5: ret = "AW_KEY_5"; break;
	case AW_KEY_EQUAL: ret = "AW_KEY_EQUAL"; break;
	case AW_KEY_9: ret = "AW_KEY_9"; break;
	case AW_KEY_7: ret = "AW_KEY_7"; break;
	case AW_KEY_MINUS: ret = "AW_KEY_MINUS"; break;
	case AW_KEY_8: ret = "AW_KEY_8"; break;
	case AW_KEY_0: ret = "AW_KEY_0"; break;
	case AW_KEY_RIGHTBRACKET: ret = "AW_KEY_RIGHTBRACKET"; break;
	case AW_KEY_O: ret = "AW_KEY_O"; break;
	case AW_KEY_U: ret = "AW_KEY_U"; break;
	case AW_KEY_LEFTBRACKET: ret = "AW_KEY_LEFTBRACKET"; break;
	case AW_KEY_I: ret = "AW_KEY_I"; break;
	case AW_KEY_P: ret = "AW_KEY_P"; break;
	case AW_KEY_L: ret = "AW_KEY_L"; break;
	case AW_KEY_J: ret = "AW_KEY_J"; break;
	case AW_KEY_QUOTE: ret = "AW_KEY_QUOTE"; break;
	case AW_KEY_K: ret = "AW_KEY_K"; break;
	case AW_KEY_SEMICOLON: ret = "AW_KEY_SEMICOLON"; break;
	case AW_KEY_BACKSLASH: ret = "AW_KEY_BACKSLASH"; break;
	case AW_KEY_COMMA: ret = "AW_KEY_COMMA"; break;
	case AW_KEY_SLASH: ret = "AW_KEY_SLASH"; break;
	case AW_KEY_N: ret = "AW_KEY_N"; break;
	case AW_KEY_M: ret = "AW_KEY_M"; break;
	case AW_KEY_PERIOD: ret = "AW_KEY_PERIOD"; break;
	case AW_KEY_GRAVE: ret = "AW_KEY_GRAVE"; break;
	case AW_KEY_KEYPADDECIMAL: ret = "AW_KEY_KEYPADDECIMAL"; break;
	case AW_KEY_KEYPADMULTIPLY: ret = "AW_KEY_KEYPADMULTIPLY"; break;
	case AW_KEY_KEYPADPLUS: ret = "AW_KEY_KEYPADPLUS"; break;
	case AW_KEY_KEYPADCLEAR: ret = "AW_KEY_KEYPADCLEAR"; break;
	case AW_KEY_KEYPADDIVIDE: ret = "AW_KEY_KEYPADDIVIDE"; break;
	case AW_KEY_KEYPADENTER: ret = "AW_KEY_KEYPADENTER"; break;
	case AW_KEY_KEYPADMINUS: ret = "AW_KEY_KEYPADMINUS"; break;
	case AW_KEY_KEYPADEQUALS: ret = "AW_KEY_KEYPADEQUALS"; break;
	case AW_KEY_KEYPAD0: ret = "AW_KEY_KEYPAD0"; break;
	case AW_KEY_KEYPAD1: ret = "AW_KEY_KEYPAD1"; break;
	case AW_KEY_KEYPAD2: ret = "AW_KEY_KEYPAD2"; break;
	case AW_KEY_KEYPAD3: ret = "AW_KEY_KEYPAD3"; break;
	case AW_KEY_KEYPAD4: ret = "AW_KEY_KEYPAD4"; break;
	case AW_KEY_KEYPAD5: ret = "AW_KEY_KEYPAD5"; break;
	case AW_KEY_KEYPAD6: ret = "AW_KEY_KEYPAD6"; break;
	case AW_KEY_KEYPAD7: ret = "AW_KEY_KEYPAD7"; break;
	case AW_KEY_KEYPAD8: ret = "AW_KEY_KEYPAD8"; break;
	case AW_KEY_KEYPAD9: ret = "AW_KEY_KEYPAD9"; break;
	case AW_KEY_RETURN: ret = "AW_KEY_RETURN"; break;
	case AW_KEY_TAB: ret = "AW_KEY_TAB"; break;
	case AW_KEY_SPACE: ret = "AW_KEY_SPACE"; break;
	case AW_KEY_DELETE: ret = "AW_KEY_DELETE"; break;
	case AW_KEY_ESCAPE: ret = "AW_KEY_ESCAPE"; break;
	case AW_KEY_COMMAND: ret = "AW_KEY_COMMAND"; break;
	case AW_KEY_SHIFT: ret = "AW_KEY_SHIFT"; break;
	case AW_KEY_CAPSLOCK: ret = "AW_KEY_CAPSLOCK"; break;
	case AW_KEY_OPTION: ret = "AW_KEY_OPTION"; break;
	case AW_KEY_CONTROL: ret = "AW_KEY_CONTROL"; break;
	case AW_KEY_RIGHTSHIFT: ret = "AW_KEY_RIGHTSHIFT"; break;
	case AW_KEY_RIGHTOPTION: ret = "AW_KEY_RIGHTOPTION"; break;
	case AW_KEY_RIGHTCONTROL: ret = "AW_KEY_RIGHTCONTROL"; break;
	case AW_KEY_FUNCTION: ret = "AW_KEY_FUNCTION"; break;
	case AW_KEY_F17: ret = "AW_KEY_F17"; break;
	case AW_KEY_VOLUMEUP: ret = "AW_KEY_VOLUMEUP"; break;
	case AW_KEY_VOLUMEDOWN: ret = "AW_KEY_VOLUMEDOWN"; break;
	case AW_KEY_MUTE: ret = "AW_KEY_MUTE"; break;
	case AW_KEY_F18: ret = "AW_KEY_F18"; break;
	case AW_KEY_F19: ret = "AW_KEY_F19"; break;
	case AW_KEY_F20: ret = "AW_KEY_F20"; break;
	case AW_KEY_F5: ret = "AW_KEY_F5"; break;
	case AW_KEY_F6: ret = "AW_KEY_F6"; break;
	case AW_KEY_F7: ret = "AW_KEY_F7"; break;
	case AW_KEY_F3: ret = "AW_KEY_F3"; break;
	case AW_KEY_F8: ret = "AW_KEY_F8"; break;
	case AW_KEY_F9: ret = "AW_KEY_F9"; break;
	case AW_KEY_F11: ret = "AW_KEY_F11"; break;
	case AW_KEY_F13: ret = "AW_KEY_F13"; break;
	case AW_KEY_F16: ret = "AW_KEY_F16"; break;
	case AW_KEY_F14: ret = "AW_KEY_F14"; break;
	case AW_KEY_F10: ret = "AW_KEY_F10"; break;
	case AW_KEY_F12: ret = "AW_KEY_F12"; break;
	case AW_KEY_F15: ret = "AW_KEY_F15"; break;
	case AW_KEY_HELP: ret = "AW_KEY_HELP"; break;
	case AW_KEY_HOME: ret = "AW_KEY_HOME"; break;
	case AW_KEY_PAGEUP: ret = "AW_KEY_PAGEUP"; break;
	case AW_KEY_FORWARDDELETE: ret = "AW_KEY_FORWARDDELETE"; break;
	case AW_KEY_F4: ret = "AW_KEY_F4"; break;
	case AW_KEY_END: ret = "AW_KEY_END"; break;
	case AW_KEY_F2: ret = "AW_KEY_F2"; break;
	case AW_KEY_PAGEDOWN: ret = "AW_KEY_PAGEDOWN"; break;
	case AW_KEY_F1: ret = "AW_KEY_F1"; break;
	case AW_KEY_LEFTARROW: ret = "AW_KEY_LEFTARROW"; break;
	case AW_KEY_RIGHTARROW: ret = "AW_KEY_RIGHTARROW"; break;
	case AW_KEY_DOWNARROW: ret = "AW_KEY_DOWNARROW"; break;
	case AW_KEY_UPARROW: ret = "AW_KEY_UPARROW"; break;
        case AW_KEY_SCROLL: ret = "AW_KEY_SCROLL"; break;
        case AW_KEY_NUMLOCK: ret = "AW_KEY_NUMLOCK"; break;
        case AW_KEY_CLEAR: ret = "AW_KEY_CLEAR"; break;
        case AW_KEY_SYSREQ: ret = "AW_KEY_SYSREQ"; break;
        case AW_KEY_PAUSE: ret = "AW_KEY_PAUSE"; break;
        case AW_KEY_CAMERA: ret = "AW_KEY_CAMERA"; break;
        case AW_KEY_CENTER: ret = "AW_KEY_CENTER"; break;
        case AW_KEY_AT: ret = "AW_KEY_AT"; break;
        case AW_KEY_SYM: ret = "AW_KEY_SYM"; break;
	default:
		if (k >= 32 && k < 127)
			snprintf(buf, sizeof(buf), "%c", (unsigned)k);
		else
			snprintf(buf, sizeof(buf), "0x%x", (unsigned)k);
		ret = buf;
	}
	return ret;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
