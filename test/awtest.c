#include <aw/sysgl.h>
#include <aw/aw.h>
#include "log.h"

static int g_exit = 0;
static const char * g_progname;

static void resize(aw * w, int ww, int wh) {
	char buf[256];
	snprintf(buf, sizeof(buf), "%s %dx%d", g_progname, ww, wh);
	awSetTitle(w, buf);
}

static void processEvents(aw * w) {
	const awEvent * awe;
	while ((awe = awNextEvent(w))) switch (awe->type) {
	case AW_EVENT_RESIZE:
		resize(w, awe->u.resize.w, awe->u.resize.h);
		Log("Resized to %d %d", awe->u.resize.w, awe->u.resize.h); break;
	case AW_EVENT_DOWN:
		Log("Down: %d", awe->u.down.which); break;
	case AW_EVENT_UP:
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
	g_progname = argv[0];
	awInit();
	w = awOpen(100, 100, 300, 400);
	if (!w) {
		Log("unable to open window (is DISPLAY set?)");
		return 1;
	}
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
   c-basic-offset: 8 **
   End: **
*/
