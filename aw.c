
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
	return awosInit();
}

void awEnd() {
	awosEnd();
}

static void show(aw * w) {
	if (check(w) && !awosShow(w))
		report("Unable to show window");
}


static void hide(aw * w) {
	if (check(w) && !awosHide(w))
		report("Unable to hide window");
}

static void setTitle(aw * w, const char * t) {
	if (check(w) && !awosSetTitle(w, t))
		report("Unable to set title");
}

aw  * awOpen(const char * t, int x, int y, int width, int height) { 
	aw * w = awosOpen(x, y, width, height, 0);
	if (!w)
		report("Unable to open window");
	if (w)
		setTitle(w, t);
	if (w)
		show(w);
	return w;
}

void awClose(aw * w) {	
	if (check(w)) {
		hide(w);
		if (!awosClose(w)) 
			report("Unable to close window");
	}
}

void awSwapBuffers(aw * w) {
	if (check(w) && !awosSwapBuffers(w))
		report("awSwapBuffers failed");
}

static void setInterval(aw * w, int interval) {
	awHeader * hdr = (awHeader*)w;
	if (check(w) && hdr->interval != interval && !awosSetSwapInterval(1)) {
		report("Unable to set swap interval");
		hdr->interval = interval;
	}
}

static void makeCurrent(void * c, void * d) {
	if (!awosMakeCurrent(c, d))
		report("Unable to establish context");
}

void awPushCurrent(aw * w) {
	if (check(w)) {
		awHeader * hdr = (awHeader*)w;
		hdr->pushctx = awosGetCurrentContext();
		hdr->pushdrw = awosGetCurrentDrawable();
		makeCurrent(awosGetContext(w), awosGetDrawable(w));
		setInterval(w, 1);
	}
}

void awPopCurrent(aw * w) {
	if (check(w)) {
		awHeader * hdr = (awHeader*)w;
		makeCurrent(hdr->pushctx, hdr->pushdrw);
	}
}


const awEvent * awNextEvent(aw * w) {
	const awEvent * ret = 0;
	awHeader * hdr = (awHeader*)w;
	if (!check(w))
		return ret;
	awosPollEvent(w);
	if (hdr->head == hdr->tail)
		return ret;
	ret = hdr->ev + hdr->tail;
	hdr->tail++;
	hdr->tail %= MAX_EVENTS;
	return ret;
}

void got(aw  * w, int type, int p1, int p2) {
	awHeader * hdr = (awHeader*)w;
	awEvent * e = hdr->ev + hdr->head;
	hdr->head++;
	hdr->head %= MAX_EVENTS;
	e->type = type;
	e->u.p[0] = p1;
	e->u.p[1] = p2;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   End: **
*/
