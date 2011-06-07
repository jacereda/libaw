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
#include "cotypes.h"
#include "co.h"
#include "bit.h"

#define PROGSTK 8*1024*1024

static co * g_mainco;
static co * g_progco;

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

static const char * coname(const co * c) {
        const char * n;
        if (c == g_mainco)
                n = "main";
        else if (c == g_progco)
                n = "prog";
        else
                n = "other";
        return n;
}

static void dbg(const char * func) {
        report("%s:%s", coname(coCurrent()), func);
}

static void unable(const char * s) {
        report("unable to %s", s);
}

static void switchTo(co * c) {
//        report("%s:switch to %s", coname(coCurrent()), coname(c));
        if (c != coCurrent()) {
        assert(c != coCurrent());
        coSwitchTo(c);
        }
//        report("%s:back to %s", coname(coCurrent()), coname(coCurrent()));
}

//#define D dbg(__func__)
#define D 

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

typedef struct _req {
        intptr_t (*f)();
        intptr_t a0;
        intptr_t a1;
        intptr_t a2;
        intptr_t a3;
        intptr_t a4;
        intptr_t a5;
        intptr_t ret;
} req;

void dispatch() {
        req * r;
        D;
        switchTo(g_progco);
        if (!progfinished()) {
                r = coData(coCurrent());
                if (r->f) 
                        r->ret = r->f(r->a0, r->a1, r->a2, r->a3, r->a4, r->a5);
                r->f = 0;
        }
}

static intptr_t cmd(req * r) {
        coSetData(g_mainco, r);
        switchTo(g_mainco);
        return r->ret;
}

static void * agnew(const char * name) {
        ag * g = calloc(1, sizeof(*g));
        osgInit(&g->osg, name);
        return g;
}

ag * agNew(const char * name) {
        req r;
        D;
        r.f = agnew;
        r.a0 = name;
        return cmd(&r);
}

void agTick(ag * g) {
}

static void * agdel(ag * g) {
        D;
        if (gcheck(g)) {
                if (!osgTerm(&g->osg))
                        unable("delege group");
                free(g);
        }
        return 0;
}

void agDel(ag * g) {
        req r;
        D;
        r.f = agdel;
        r.a0 = g;
        return cmd(&r);
}

static void * awshow(aw * w) {
        D;
        if (wcheck(w)) {
                if (!oswShow(&w->osw))
                        unable("show window");
                else
                        w->shown = 1;
        }
        return 0;
}

void awShow(aw * w) {
        req r;
        D;
        r.f = awshow;
        r.a0 = w;
        cmd(&r);
}

static void * awhide(aw * w) {
        D;
        if (wcheck(w)) {
                if (!oswHide(&w->osw))
                        unable("hide window");
                else
                        w->shown = 0;
        }
        return 0;
}

void awHide(aw * w) {
        req r;
        D;
        r.f = awhide;
        r.a0 = w;
        cmd(&r);
}

static void awsettitle(aw * w, const char * t) {
        if (wcheck(w) && !w->maximized && !oswSetTitle(&w->osw, t))
                unable("set window title");
}

void awSetTitle(aw * w, const char * t) {
        req r;
        D;
        r.f = awsettitle;
        r.a0 = w;
        r.a1 = t;
        cmd(&r);
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

static intptr_t awnextevent(aw * w) {
        const ae * e = 0;
        D;
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

static void drain(aw * w) {
        while (awnextevent(w))
                ;
}

#define BADPOS 123
#define BADSIZE 17

static int wopen(ag * g, aw * w) {
        int ok;
        D;
        memset(&w->osw, 0, sizeof(w->osw));
        w->g = g;
        w->last = w->ev;
        w->last->type = AW_EVENT_NONE;
        w->head = w->tail = 0;
        D;
        ok = oswInit(&w->osw, &g->osg, w->rx, w->ry, w->rw, w->rh, !w->borders);
        D;
        drain(w);
        D;
        // horrible hack to workaround lack of notifications in Cocoa
        oswGeometry(&w->osw, w->rx+1, w->ry+1, w->rw+1, w->rh+1);
        D;
        drain(w);
        if (!ok)
                unable("open window");
        return ok;
}

static void wclose(aw * w) {
        awPointer(w, 0);
        if (w->ctx)
                oswClearCurrent(&w->osw);
        oswHide(&w->osw);
        drain(w);
        if (!oswTerm(&w->osw))
                unable("close window");
        memset(&w->osw, 0, sizeof(w->osw));
}

static int wreopen(aw * w) {
        int ok;
        ac * c = w->ctx;
        wclose(w);
        w->ctx = 0;
        ok = wopen(w->g, w);
        w->ctx = c;
        if (w->ctx)
                oswMakeCurrent(&w->osw, &w->ctx->osc);
        drain(w);
        if (w->maximized)
                oswMaximize(&w->osw);
        else
                oswGeometry(&w->osw, w->rx, w->ry, w->rw, w->rh);
        if (w->shown)
                oswShow(&w->osw);
//        filterBad(w);
        return ok;
}


intptr_t awnew(ag * g) {
        aw * w = calloc(1, sizeof(*w));
        int ok;
        D;
        w->rx = 31;
        w->ry = 31;
        w->rw = 31;
        w->rh = 31;
        w->borders = 1;
        ok = wopen(g, w);
        if (!ok) {
                free(w);
                w = 0;
        }
        return w;
}


aw * awNew(ag * g) { 
        req r;
        D;
        r.f = awnew; r.a0 = g;
        return cmd(&r);
}

static intptr_t awdel(aw * w) {
        D;
        if (wcheck(w)) {
                if (w->pointer) 
                        report("closing window with cursor established");
                if (w->ctx) 
                        report("closing window with active context");
                wclose(w);
        }
        return 0;
}

void awDel(aw * w) {  
        req r;
        D;
        r.f = awdel; r.a0 = w;
        cmd(&r);
}

static intptr_t awshowborders(aw * w) {
        D;
        if (wcheck(w)) {
                w->borders = 1;
                wreopen(w);
        }
        return 0;
}

void awShowBorders(aw * w) {
        req r;
        D;
        r.f = awshowborders; r.a0 = w;
        cmd(&r);
}

static intptr_t awhideborders(aw * w) {
        D;
        if (wcheck(w)) {
                w->borders = 0;
                wreopen(w);
        }
        return 0;
}

void awHideBorders(aw * w) {
        req r;
        D;
        r.f = awhideborders; r.a0 = w;
        cmd(&r);
}

static intptr_t awmaximize(aw * w) {
        D;
        if (wcheck(w)) {
                w->maximized = 1;
                wreopen(w);
        }
        return 0;
}

void awMaximize(aw * w) {
        req r;
        D;
        r.f = awmaximize; r.a0 = w;
        cmd(&r);
}

static intptr_t awnormalize(aw * w) {
        D;
        if (wcheck(w)) {
                w->maximized = 0;
                wreopen(w);
        }
        return 0;
}

void awNormalize(aw * w) {
        req r;
        D;
        r.f = awnormalize; r.a0 = w;
        cmd(&r);
}

static void setInterval(aw * w) {
        if (w->interval && !oswSetSwapInterval(&w->osw, w->interval)) 
                unable("set swap interval");
}

static intptr_t awswapbuffers(aw * w) {
        D;
        if (wcheck(w)) {
                setInterval(w);
                if (!w->ctx)
                        unable("swap buffers without context");
                else {
                        glFlush();
                        if (!oswSwapBuffers(&w->osw))
                                unable("swap buffers");
                }
                memcpy(w->ppressed, w->pressed, sizeof(w->ppressed));
//                switchTo(g_mainco);
        }
        return 0;
}

void awSwapBuffers(aw * w) {
        req r;
        D;
        r.f = awswapbuffers; r.a0 = w;
        cmd(&r);
}

static void awmakecurrent(aw * w, ac * c) {
        D;
        if (wcheck(w)) {
                if (w->ctx)
                        glFlush();
                if (w->ctx && w->ctx != c) 
                        oswClearCurrent(&w->osw);
                if (c && !oswMakeCurrent(&w->osw, &c->osc))
                        unable("establish context");
                w->ctx = c;
        }
}

void awMakeCurrent(aw * w, ac * c) {
        req r;
        D;
        r.f = awmakecurrent; r.a0 = w; r.a1 = c;
        cmd(&r);
}

const ae * awNextEvent(aw * w) {
        req r;
        D;
        r.f = awnextevent; r.a0 = w;
        return cmd(&r);
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

static int pressed(const aw * w, awkey k) {
        const unsigned char * ba = bitarrayForC(w->pressed, &k);
        return ba && bittest(ba, k);
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

void awShowKeyboard(aw * w) {
        if (!oswShowKeyboard(&w->osw))
                unable("show keyboard");
}

void awHideKeyboard(aw * w) {
        if (!oswHideKeyboard(&w->osw))
                unable("hide keyboard");
}

void got(osw * osw, int type, intptr_t p1, intptr_t p2) {
        aw * w = (aw*)osw;
        ae * e = w->ev + w->head;
        D;
        assert(w);
        w->head++;
        w->head %= MAX_EVENTS;
        e->type = type;
        e->p[0] = p1;
        e->p[1] = p2;
}

intptr_t acnew(ag * g, ac * share) {
        ac * c = calloc(1, sizeof(*c));
        int ok;
        D;
        c->g = g;
        ok = oscInit(&c->osc, &g->osg, &share->osc);
        if (!ok) {
                unable("create context");
                free(c);
                c = 0;
        }
        return c;
}

ac * acNew(ag * g, ac * share) {
        req r;
        D;
        r.f = acnew; r.a0 = g; r.a1 = share;
        return cmd(&r);
}

ac * acNewStereo(ag * g, ac * share) {
        D;
        return 0; // XXX
}

static intptr_t acdel(ac * c) {
        D;
        if (ccheck(c)) {
                if (!oscTerm(&c->osc))
                        unable("delete context");
                free(c);
        }
        return 0;
}

void acDel(ac * c) {
        req r;
        D;
        r.f = acdel; r.a0 = c;
        cmd(&r);
}

void awSetInterval(aw * w, int interval) {
        D;
        w->interval = interval;
}

void awSetUserData(aw * w, void * user) {
        D;
        w->user = user;
}

void * awUserData(const aw * w) {
        D;
        return w->user;
}

static intptr_t awgeometry(aw * w, int x, int y,
                           unsigned width, unsigned height) {
        D;
        if (wcheck(w)) {
                if (!oswGeometry(&w->osw, x, y, width, height))
                        unable("establish geometry");
                else {
                        w->rx = x;
                        w->ry = y;
                        w->rw = width;
                        w->rh = height;
                }
        }
        return 0;
}

void awGeometry(aw * w, int x, int y, unsigned width, unsigned height) {
        req r;
        D;
        r.f = awgeometry; r.a0 = w; r.a1 = x; r.a2 = y; 
        r.a3 = width; r.a4 = height;
        cmd(&r);
}

static intptr_t awpointer(aw * w, ap * p) {
        D;
        if (w->pointer)
                w->pointer->refs--;
        w->pointer = p;
        if (w->pointer)
                w->pointer->refs++;
        oswPointer(&w->osw);
        return 0;
}

void awPointer(aw * w, ap * p) {
        req r;
        D;
        r.f = awpointer; r.a0 = w; r.a1 = p;
        cmd(&r);
}

intptr_t apnew(const void * rgba, uintptr_t hotx, uintptr_t hoty) {
        ap * p = calloc(1, sizeof(*p));
        int ok;
        D;
        ok = ospInit(&p->osp, rgba, hotx, hoty);
        if (!ok) {
                unable("create pointer");
                free(p);
                p = 0;
        }
        return p;
}

ap * apNew(const void * rgba, unsigned hotx, unsigned hoty) {
        req r;
        D;
        r.f = apnew; r.a0 = rgba; r.a1 = hotx; r.a2 = hoty;
        return cmd(&r);
}

static intptr_t apdel(ap * p) {
        D;
        if (p->refs)
                unable("destroy referenced pointer");
        else if (pcheck(p)) {
                if (!ospTerm(&p->osp))
                        unable("destroy pointer");
                free(p);
        }
        return 0;
}

void apDel(ap * p) {
        req r;
        D;
        r.f = apdel; r.a0 = p;
        cmd(&r);
}

static intptr_t aworder(const aw * w) {
        aw * zorder[MAX_WINDOWS];
        unsigned n;
        unsigned i = 0;
        D;
        n = oswOrder((osw**)zorder);
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

unsigned awOrder(const aw * w) {
        req r;
        D;
        r.f = aworder; r.a0 = w;
        return cmd(&r);
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
	const char * n = 0;
	static char buf[32] = {0};
        awkey k = aeWhich(e);
	switch (k) {
	case AW_KEY_NONE: n = "NONE"; break;
	case AW_KEY_MOUSEWHEELUP: n = "MOUSEWHEELUP"; break;
	case AW_KEY_MOUSEWHEELDOWN: n = "MOUSEWHEELDOWN"; break;
	case AW_KEY_MOUSELEFT: n = "MOUSELEFT"; break;
	case AW_KEY_MOUSEMIDDLE: n = "MOUSEMIDDLE"; break;
	case AW_KEY_MOUSERIGHT: n = "MOUSERIGHT"; break;
	case AW_KEY_A: n = "A"; break;
	case AW_KEY_S: n = "S"; break;
	case AW_KEY_D: n = "D"; break;
	case AW_KEY_F: n = "F"; break;
	case AW_KEY_H: n = "H"; break;
	case AW_KEY_G: n = "G"; break;
	case AW_KEY_Z: n = "Z"; break;
	case AW_KEY_X: n = "X"; break;
	case AW_KEY_C: n = "C"; break;
	case AW_KEY_V: n = "V"; break;
	case AW_KEY_B: n = "B"; break;
	case AW_KEY_Q: n = "Q"; break;
	case AW_KEY_W: n = "W"; break;
	case AW_KEY_E: n = "E"; break;
	case AW_KEY_R: n = "R"; break;
	case AW_KEY_Y: n = "Y"; break;
	case AW_KEY_T: n = "T"; break;
	case AW_KEY_1: n = "1"; break;
	case AW_KEY_2: n = "2"; break;
	case AW_KEY_3: n = "3"; break;
	case AW_KEY_4: n = "4"; break;
	case AW_KEY_6: n = "6"; break;
	case AW_KEY_5: n = "5"; break;
	case AW_KEY_EQUAL: n = "EQUAL"; break;
	case AW_KEY_9: n = "9"; break;
	case AW_KEY_7: n = "7"; break;
	case AW_KEY_MINUS: n = "MINUS"; break;
	case AW_KEY_8: n = "8"; break;
	case AW_KEY_0: n = "0"; break;
	case AW_KEY_RIGHTBRACKET: n = "RIGHTBRACKET"; break;
	case AW_KEY_O: n = "O"; break;
	case AW_KEY_U: n = "U"; break;
	case AW_KEY_LEFTBRACKET: n = "LEFTBRACKET"; break;
	case AW_KEY_I: n = "I"; break;
	case AW_KEY_P: n = "P"; break;
	case AW_KEY_L: n = "L"; break;
	case AW_KEY_J: n = "J"; break;
	case AW_KEY_QUOTE: n = "QUOTE"; break;
	case AW_KEY_K: n = "K"; break;
	case AW_KEY_SEMICOLON: n = "SEMICOLON"; break;
	case AW_KEY_BACKSLASH: n = "BACKSLASH"; break;
	case AW_KEY_COMMA: n = "COMMA"; break;
	case AW_KEY_SLASH: n = "SLASH"; break;
	case AW_KEY_N: n = "N"; break;
	case AW_KEY_M: n = "M"; break;
	case AW_KEY_PERIOD: n = "PERIOD"; break;
	case AW_KEY_GRAVE: n = "GRAVE"; break;
	case AW_KEY_KEYPADDECIMAL: n = "KEYPADDECIMAL"; break;
	case AW_KEY_KEYPADMULTIPLY: n = "KEYPADMULTIPLY"; break;
	case AW_KEY_KEYPADPLUS: n = "KEYPADPLUS"; break;
	case AW_KEY_KEYPADCLEAR: n = "KEYPADCLEAR"; break;
	case AW_KEY_KEYPADDIVIDE: n = "KEYPADDIVIDE"; break;
	case AW_KEY_KEYPADENTER: n = "KEYPADENTER"; break;
	case AW_KEY_KEYPADMINUS: n = "KEYPADMINUS"; break;
	case AW_KEY_KEYPADEQUALS: n = "KEYPADEQUALS"; break;
	case AW_KEY_KEYPAD0: n = "KEYPAD0"; break;
	case AW_KEY_KEYPAD1: n = "KEYPAD1"; break;
	case AW_KEY_KEYPAD2: n = "KEYPAD2"; break;
	case AW_KEY_KEYPAD3: n = "KEYPAD3"; break;
	case AW_KEY_KEYPAD4: n = "KEYPAD4"; break;
	case AW_KEY_KEYPAD5: n = "KEYPAD5"; break;
	case AW_KEY_KEYPAD6: n = "KEYPAD6"; break;
	case AW_KEY_KEYPAD7: n = "KEYPAD7"; break;
	case AW_KEY_KEYPAD8: n = "KEYPAD8"; break;
	case AW_KEY_KEYPAD9: n = "KEYPAD9"; break;
	case AW_KEY_RETURN: n = "RETURN"; break;
	case AW_KEY_TAB: n = "TAB"; break;
	case AW_KEY_SPACE: n = "SPACE"; break;
	case AW_KEY_DELETE: n = "DELETE"; break;
	case AW_KEY_ESCAPE: n = "ESCAPE"; break;
	case AW_KEY_COMMAND: n = "COMMAND"; break;
	case AW_KEY_SHIFT: n = "SHIFT"; break;
	case AW_KEY_CAPSLOCK: n = "CAPSLOCK"; break;
	case AW_KEY_OPTION: n = "OPTION"; break;
	case AW_KEY_CONTROL: n = "CONTROL"; break;
	case AW_KEY_RIGHTSHIFT: n = "RIGHTSHIFT"; break;
	case AW_KEY_RIGHTOPTION: n = "RIGHTOPTION"; break;
	case AW_KEY_RIGHTCONTROL: n = "RIGHTCONTROL"; break;
	case AW_KEY_FUNCTION: n = "FUNCTION"; break;
	case AW_KEY_F17: n = "F17"; break;
	case AW_KEY_VOLUMEUP: n = "VOLUMEUP"; break;
	case AW_KEY_VOLUMEDOWN: n = "VOLUMEDOWN"; break;
	case AW_KEY_MUTE: n = "MUTE"; break;
	case AW_KEY_F18: n = "F18"; break;
	case AW_KEY_F19: n = "F19"; break;
	case AW_KEY_F20: n = "F20"; break;
	case AW_KEY_F5: n = "F5"; break;
	case AW_KEY_F6: n = "F6"; break;
	case AW_KEY_F7: n = "F7"; break;
	case AW_KEY_F3: n = "F3"; break;
	case AW_KEY_F8: n = "F8"; break;
	case AW_KEY_F9: n = "F9"; break;
	case AW_KEY_F11: n = "F11"; break;
	case AW_KEY_F13: n = "F13"; break;
	case AW_KEY_F16: n = "F16"; break;
	case AW_KEY_F14: n = "F14"; break;
	case AW_KEY_F10: n = "F10"; break;
	case AW_KEY_F12: n = "F12"; break;
	case AW_KEY_F15: n = "F15"; break;
	case AW_KEY_HELP: n = "HELP"; break;
	case AW_KEY_HOME: n = "HOME"; break;
	case AW_KEY_PAGEUP: n = "PAGEUP"; break;
	case AW_KEY_FORWARDDELETE: n = "FORWARDDELETE"; break;
	case AW_KEY_F4: n = "F4"; break;
	case AW_KEY_END: n = "END"; break;
	case AW_KEY_F2: n = "F2"; break;
	case AW_KEY_PAGEDOWN: n = "PAGEDOWN"; break;
	case AW_KEY_F1: n = "F1"; break;
	case AW_KEY_LEFTARROW: n = "LEFTARROW"; break;
	case AW_KEY_RIGHTARROW: n = "RIGHTARROW"; break;
	case AW_KEY_DOWNARROW: n = "DOWNARROW"; break;
	case AW_KEY_UPARROW: n = "UPARROW"; break;
        case AW_KEY_SCROLL: n = "SCROLL"; break;
        case AW_KEY_NUMLOCK: n = "NUMLOCK"; break;
        case AW_KEY_CLEAR: n = "CLEAR"; break;
        case AW_KEY_SYSREQ: n = "SYSREQ"; break;
        case AW_KEY_PAUSE: n = "PAUSE"; break;
        case AW_KEY_CAMERA: n = "CAMERA"; break;
        case AW_KEY_CENTER: n = "CENTER"; break;
        case AW_KEY_AT: n = "AT"; break;
        case AW_KEY_SYM: n = "SYM"; break;
	default: n = 0; break;
        }
        if (n)
                snprintf(buf, sizeof(buf), "AW_KEY_%s", n);
        else if (k >= 32 && k < 127)
                snprintf(buf, sizeof(buf), "%c", (unsigned)k);                
        else
                snprintf(buf, sizeof(buf), "0x%x", (unsigned)k);
	return buf;
}

struct args {
        int argc;
        char ** argv;
        int ret;
};

static void prgentry(void * vargs) {
        extern int fakemain(int, char **);
        struct args * a = (struct args *)vargs;
        a->ret = fakemain(a->argc, a->argv);
        while (1)
                switchTo(g_mainco);
}

struct args g_a;

int progfinished() {
        return g_a.ret != 0xdeadbeef;
}

void progrun(int argc, char ** argv) {
        g_a.argc = argc;
        g_a.argv = argv;
        g_a.ret = 0xdeadbeef;
        D;
        g_mainco = coMain(0);
        g_progco = coNew(prgentry, &g_a, PROGSTK);
}

int progterm() {
        coDel(g_progco);
        coDel(g_mainco);
        return g_a.ret;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
