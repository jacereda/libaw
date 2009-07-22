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

void report(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "AW ERROR: ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	fflush(stderr);
}

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
	if (*k < MAX_PRESSED)
		return hdr->pressed;
	*k -= AW_KEY_NONE;
	if (*k < MAX_SPRESSED)
		return hdr->spressed;
	return 0;
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

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
