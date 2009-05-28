
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

aw * awOpen(int x, int y, int w, int h) { 
	return open(x, y, w, h, 0, 0);
}

aw * awOpenBorderless(int x, int y, int w, int h) { 
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
		awHeader * hdr = (acHeader*)w;
		if (!hdr->ctx)
			report("awSwapBuffers called without context");
		else if (!awosSwapBuffers(w))
			report("awSwapBuffers failed");
	}
}

static void setInterval(aw * w, ac * c) {
	acHeader * hdr = (acHeader*)c;
	if (!awosSetSwapInterval(w, hdr->interval)) 
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

const awEvent * awNextEvent(aw * w) {
	const awEvent * awe = 0;
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
