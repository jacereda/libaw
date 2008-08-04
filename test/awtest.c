#include <stdarg.h>
#include <stdio.h>
#include <aw/sysgl.h>
#include <aw/aw.h>

static int g_exit = 0;
static int g_y = 0;
static int g_compressed = 0;

static void Log(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	fflush(stdout);
	va_end(ap);
}

static const awEvent * nextEvent(aw * w) {
	const awEvent * ret;
	if (g_compressed)
		ret = awCompressedNextEvent(w);
	else
		ret = awNextEvent(w);		
	return ret;
}

static void processEvents(aw * w) {
	const awEvent * awe;
	while ((awe = nextEvent(w))) switch (awe->type) {
	case AW_EVENT_MOVE:
		Log("Moved to %d %d", awe->u.move.x, awe->u.move.y); break;
	case AW_EVENT_RESIZE:
		Log("Resized to %d %d", awe->u.resize.w, awe->u.resize.h); break;
	case AW_EVENT_DOWN:
		Log("Down: %d", awe->u.down.which); break;
	case AW_EVENT_UP:
		if (awe->u.up.which == AW_KEY_MOUSEMIDDLE) {
			g_compressed = 1 - g_compressed;
			Log("Compressed: %d", g_compressed);
			}
		Log("Up: %d", awe->u.up.which); break;
	case AW_EVENT_MOTION:
		Log("Motion: %d,%d", awe->u.motion.x, awe->u.motion.y); break;
	case AW_EVENT_CLOSE:
		Log("Exit requested");
		g_exit = 1; break;
	}
}

static void draw() {
	glClearColor(0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main(int argc, char ** argv) {
	aw * w;
	awInit();
	w = awOpen();
	if (!w) {
		Log("unable to open window (is DISPLAY set?)");
		return 1;
	}
		
	processEvents(w);
	awMove(w, 100, 200);
	processEvents(w);
	awResize(w, 200, 300);
	processEvents(w);
	awSetTitle(w, "foo");
	processEvents(w);
	awShow(w);
	processEvents(w);
	awHide(w);
	processEvents(w);
	awShow(w);
	processEvents(w);
	awMove(w, 300, 400);
	processEvents(w);
	awMove(w, 0, 50);
	processEvents(w);
	awPushCurrent(w);
	while (!g_exit) {
		draw();
		awSwapBuffers(w);
		processEvents(w);
	}
	awPopCurrent(w);
	awClose(w);
	awEnd();
	return 0;
}

/* 
Local variables: **
c-file-style: "bsd" **
End: **
*/
