#include <aw/sysgl.h>
#include <aw/aw.h>
#include "log.h"

static int g_exit;
static const char * g_progname;

// To test the browser plugin
EXPORTED void hello() {
	Log("Hello World!");
}

static void resize(aw * w, int ww, int wh) {
	char buf[256] = {0};
	snprintf(buf, sizeof(buf), "%s %dx%d", g_progname, ww, wh);
	awSetTitle(w, buf);
}

static aw * processEvents(aw * w) {
	const awEvent * awe;
	while ((awe = awNextEvent(w))) switch (awe->type) {
	case AW_EVENT_RESIZE:
		resize(w, awe->u.resize.w, awe->u.resize.h);
		Log("Resized to %d %d", awe->u.resize.w, awe->u.resize.h); 
		break;
	case AW_EVENT_UNICODE:
		Log("Unicode: %s", awKeyName(awe->u.unicode.which));
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
		Log("Down: %s", awKeyName(awe->u.down.which));
		break;
	case AW_EVENT_UP:
		Log("Up: %s", awKeyName(awe->u.up.which));
		break;
	case AW_EVENT_MOTION:
		Log("Motion: %d,%d", awe->u.motion.x, awe->u.motion.y); 
		break;
	case AW_EVENT_CLOSE:
		Log("Exit requested");
		g_exit = 1; 
		break;
	default: break;
	}
	if (awPressed(w, AW_KEY_A))
		Log("a pressed");
	return w;
}

static void draw() {
	static int i = 0;
	glClearColor((i++ & 0xff) / 255.0, 0, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
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
	awMakeCurrent(w, c);
	g_exit = 0;
	while (!g_exit) {
		w = processEvents(w);
		awMakeCurrent(w, c);
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
