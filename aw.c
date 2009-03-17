
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


static aw * g_share = 0;

static int check(const aw * w) {
		if (!w)
				report("Null handle");
		return w != 0;
}

int awInit() {
		int ret = awosInit();
		g_share = awOpen(0, 0, 1, 1);
		return ret;
}

void awEnd() {
		awClose(g_share); g_share = 0;
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

void awSetTitle(aw * w, const char * t) {
		if (check(w) && !awosSetTitle(w, t))
				report("Unable to set window title");
}

aw	* awOpen(int x, int y, int width, int height) { 
		void * share = g_share? awosGetContext(g_share) : 0;
		aw * w = awosOpen(x, y, width, height, share);
		if (!w)
				report("Unable to open window");
		if (w && g_share)
				show(w);
		if (!g_share && w)
				g_share = awosGetContext(w);
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

void got(aw	 * w, int type, int p1, int p2) {
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
   c-basic-offset: 8 **
   End: **
*/
