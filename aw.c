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
#include "aw.h"
#include "awos.h"

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif
#if defined(AWPLUGIN)
void report(const char *fmt, ...) {
#if 1 //defined(_WIN32)
	FILE *out = fopen("c:\\Users\\Jorge Acereda\\libaw\\aw.log", "a");
#else
	FILE *out = fopen("/tmp/aw.log", "a");
#endif
	va_list ap;
	va_start(ap, fmt);
	if(out) {
		vfprintf(out, fmt, ap);
		fputs("\n", out);
		fclose(out);
	}
	va_end(ap);
}
#else
void report(const char * fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "AW ERROR: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
        fflush(stderr);
}
#endif

static void bitset(unsigned char * b, unsigned bit) {
        b[bit>>3] |= 1 << (bit & 7);
}

static void bitclear(unsigned char * b, unsigned bit) {
        b[bit>>3] &= ~(1 << (bit & 7));
}

static int bittest(unsigned char * b, unsigned bit) {
        return b[bit>>3] & (1 << (bit & 7));
}

static int check(const aw * w) {
        if (!w)
                report("Null handle");
        return w != 0;
}

int awInit() {
        int ret = awosInit();
        if (!ret)
                report("initializing aw");
        return ret;
}

void awEnd() {
        if (!awosEnd())
                report("terminating aw");
}

static void show(aw * w) {
        if (check(w) && !awosShow(w))
                report("Unable to show window");
}


static void hide(aw * w) {
        if (check(w) && !awosHide(w))
                report("Unable to hide window");
}

void awSetTitle(aw * w, const char * t) {
        awHeader * hdr = (awHeader*)w;
        if (check(w) && !hdr->fullscreen && !awosSetTitle(w, t))
                report("Unable to set window title");
}

static aw * open(int x, int y, int width, int height, int fs, int bl) {
        aw * w = awosOpen(x, y, width, height, fs, bl);
        if (!w)
                report("Unable to open window");
        if (w)
                ((awHeader*)w)->fullscreen = fs;
        if (w)
                show(w);
        return w;
}

aw * awOpen(int x, int y, unsigned w, unsigned h) { 
        return open(x, y, w, h, 0, 0);
}

aw * awOpenBorderless(int x, int y, unsigned w, unsigned h) { 
        return open(x, y, w, h, 0, 1);
}

aw * awOpenFullscreen() {
        return open(0, 0, 0, 0, 1, 1);
}

aw * awOpenMaximized() {
        return open(0, 0, 0, 0, 1, 0);
}

void awClose(aw * w) {  
        if (check(w)) {
                if (((awHeader*)w)->ctx) {
                        report("Closing window with active context");
                        awMakeCurrent(w, 0);
                }
                hide(w);
                if (!awosClose(w))
                        report("Unable to close window");
        }
}

void awSwapBuffers(aw * w) {
        if (check(w)) {
                awHeader * hdr = (awHeader*)w;
                if (!hdr->ctx)
                        report("awSwapBuffers called without context");
                else if (!awosSwapBuffers(w))
                        report("awSwapBuffers failed");
        }
}

static void setInterval(aw * w, ac * c) {
        acHeader * hdr = (acHeader*)c;
        if (hdr->interval && !awosSetSwapInterval(w, hdr->interval)) 
                report("Unable to set swap interval");
}

void awMakeCurrent(aw * w, ac * c) {
        awHeader * hdr = (awHeader*)w;
        if (check(w)) {
                if (hdr->ctx && hdr->ctx != c)
                        awosClearCurrent(w);
                if (c && !awosMakeCurrent(w, c))
                        report("Unable to establish context");
                hdr->ctx = c;
                if (c)
                        setInterval(w, c);
        }
}

static unsigned char * bitarrayFor(awHeader * hdr, unsigned * k) {
        int good = *k >= AW_KEY_NONE && *k < AW_KEY_MAX;
        assert(good);
        *k -= AW_KEY_NONE;
        return good? hdr->pressed : 0;
}

static int pressed(awHeader * hdr, unsigned k) {
        unsigned char * ba = bitarrayFor(hdr, &k);
        return ba && bittest(ba, k);
}

static void press(awHeader * hdr, unsigned k) {
        unsigned char * ba = bitarrayFor(hdr, &k);
        if (ba)
                bitset(ba, k);
}

static void release(awHeader * hdr, unsigned k) {
        unsigned char * ba = bitarrayFor(hdr, &k);
        if (ba)
                bitclear(ba, k);    
}

const awEvent * awNextEvent(aw * w) {
        const awEvent * awe = 0;
        static const awEvent none = {AW_EVENT_NONE};
        awHeader * hdr = (awHeader*)w;
        if (!check(w))
                return 0;
        awosPollEvent(w);
        if (hdr->head != hdr->tail) {
                awe = hdr->ev + hdr->tail;
                hdr->tail++;
                hdr->tail %= MAX_EVENTS;
        }
        if (awe) switch (awe->type) {   
                case AW_EVENT_RESIZE:
                        hdr->width = awe->u.resize.w;
                        hdr->height = awe->u.resize.h;
                        break;
                case AW_EVENT_MOTION:
                        hdr->mx = awe->u.motion.x;
                        hdr->my = awe->u.motion.y;
                        break;
                case AW_EVENT_DOWN:
                        if (pressed(hdr, awe->u.down.which))
                                awe = &none;
                        else
                                press(hdr, awe->u.down.which);
                        break;
                case AW_EVENT_UP:
                        release(hdr, awe->u.up.which);
                        break;
                default: break;
                }
        return awe;
}

unsigned awWidth(aw * w) {
        awHeader * hdr = (awHeader*)w;
        return hdr->width;
}

unsigned awHeight(aw * w) {
        awHeader * hdr = (awHeader*)w;
        return hdr->height;
}

int awMouseX(aw * w) {
        awHeader * hdr = (awHeader*)w;
        return hdr->mx;
}

int awMouseY(aw * w) {
        awHeader * hdr = (awHeader*)w;
        return hdr->my;
}

int awPressed(aw * w, unsigned k) {
        awHeader * hdr = (awHeader*)w;
        unsigned char * ba = bitarrayFor(hdr, &k);
        return ba && bittest(ba, k);
}

void got(aw * w, int type, int p1, int p2) {
        awHeader * hdr = (awHeader*)w;
        awEvent * e = hdr->ev + hdr->head;
        hdr->head++;
        hdr->head %= MAX_EVENTS;
        e->type = type;
        e->u.p[0] = p1;
        e->u.p[1] = p2;
}

ac * acNew(ac * share) {
        ac * ret = acosNew(share);
        if (!ret)
                report("unable to create context sharing with %p", share);
        else
                ((acHeader*)ret)->interval = 0;
        return ret;
}

void acDel(ac * c) {
        if (!acosDel(c))
                report("unable to delete context %p", c);
}

void acSetInterval(ac * c, int interval) {
        acHeader * hdr = (acHeader*)c;
        hdr->interval = interval;
}

const char * awKeyName(unsigned k) {
	const char * ret = 0;
	static char buf[16] = {0};
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
	default:
		if (k >= 32 && k < 127)
			snprintf(buf, sizeof(buf), "%c", k);
		else
			snprintf(buf, sizeof(buf), "0x%x", k);
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
