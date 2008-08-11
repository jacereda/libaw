
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "aw.h"
#include "awos.h"

void report(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "ERROR: ");
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

aw  * awOpen() {
	return awOpenSharing(0);
}

aw  * awOpenSharing(void * ctx) {
	aw * w = awosOpen("AW Window", 100, 100, 64, 64, ctx);
	if (!w)
		report("Unable to open window");
	return w;
}

void awClose(aw * w) {
	if (check(w) && !awosClose(w)) 
		report("Unable to close window");
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

void awPushCurrent(aw * w) {
	if (check(w) && !awosPushCurrent(w))
		report("awPushCurrent failed");
	setInterval(w, 1);
}

void awPopCurrent(aw * w) {
	if (check(w) && !awosPopCurrent(w))
		report("awPopCurrent failed");
}

void awShow(aw * w) {
	if (check(w) && !awosShow(w))
		report("awShow failed");
}

void awHide(aw * w) {
	if (check(w) && !awosHide(w))
		report("awHide failed");
}

void awSetTitle(aw * w, const char * t) {
	if (check(w) && !awosSetTitle(w, t))
		report("awSetTitle failed");
}

void awResize(aw * w, int width, int height) {
	if (check(w) && !awosResize(w, width, height))
		report("awResize failed");
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
	report("queue: %d %d", hdr->head, hdr->tail);
	e->type = type;
	e->u.p[0] = p1;
	e->u.p[1] = p2;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   End: **
*/
