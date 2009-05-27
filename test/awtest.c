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

static const char * keyName(int k) {
	const char * ret = 0;
	static char buf[2] = {0};
	switch (k) {
	case AW_KEY_NONE: ret = "NONE"; break;
	case AW_KEY_MOUSEWHEELUP: ret = "MOUSEWHEELUP"; break;
	case AW_KEY_MOUSEWHEELDOWN: ret = "MOUSEWHEELDOWN"; break;
	case AW_KEY_MOUSELEFT: ret = "MOUSELEFT"; break;
	case AW_KEY_MOUSEMIDDLE: ret = "MOUSEMIDDLE"; break;
	case AW_KEY_MOUSERIGHT: ret = "MOUSERIGHT"; break;
	case AW_KEY_SHIFT: ret = "SHIFT"; break;
	case AW_KEY_ALT: ret = "ALT"; break;
	case AW_KEY_CONTROL: ret = "CONTROL"; break;
	case AW_KEY_META: ret = "META"; break;
	default:
		if (k >= 32 && k < 256) {
			buf[0] = k;
			ret = buf;
		}
		else
			ret = "unknown";
	}
	return ret;
}

static aw * processEvents(aw * w) {
	const awEvent * awe;
	while ((awe = awNextEvent(w))) switch (awe->type) {
	case AW_EVENT_RESIZE:
		resize(w, awe->u.resize.w, awe->u.resize.h);
		Log("Resized to %d %d", awe->u.resize.w, awe->u.resize.h); 
		break;
	case AW_EVENT_DOWN:
		if (awe->u.down.which == 'f') {
			awClose(w);
			w = awOpenFullscreen();
		}
		if (awe->u.down.which == 'b') {
			awClose(w);
			w = awOpenBorderless(100, 100, 300, 400);
		}
		if (awe->u.down.which == 'w') {
			awClose(w);
			w = awOpen(100, 100, 300, 400);
		}
		if (awe->u.down.which == 'm') {
			awClose(w);
			w = awOpenMaximized();
		}
		if (awe->u.down.which == 'q') 
			g_exit = 1;
		Log("Down: %s", keyName(awe->u.down.which));
		break;
	case AW_EVENT_UP:
		Log("Up: %s", keyName(awe->u.down.which));
		break;
	case AW_EVENT_MOTION:
		Log("Motion: %d,%d", awe->u.motion.x, awe->u.motion.y); 
		break;
	case AW_EVENT_CLOSE:
		Log("Exit requested");
		g_exit = 1; 
		break;
	}
	return w;
}

static void draw() {
	glClearColor(0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main(int argc, char ** argv) {
	aw * w = 0;
	ac * c = 0;
	g_progname = argv[0];
	if (awInit())
		c = acNew(0);
	if (c)
		w = awOpen(10, 10, 300, 400);
	if (!w) {
		Log("unable to open window (is DISPLAY set?)");
		return 1;
	}
	acSetInterval(c, 1);
	while (!g_exit) {
		awMakeCurrent(w, c);
		w = processEvents(w);
		draw();
		awSwapBuffers(w);
	}
	awMakeCurrent(w, 0);
	acDel(c);
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
