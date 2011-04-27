#include <aw/sysgl.h>
#include <aw/aw.h>
#include "log.h"

static int g_exit = 0;

static void processEvents(aw * w, ac * c, int n) {
	const awEvent * awe;
	while ((awe = awNextEvent(w))) switch (awe->type) {
	case AW_EVENT_RESIZE:
		Log("Resized %d: %d %d", 
		    n, awe->u.resize.w, awe->u.resize.h); break;
	case AW_EVENT_DOWN:
		Log("Down %d: %d", n, awe->u.down.which); break;
	case AW_EVENT_UP:
		Log("Up %d: %d", n, awe->u.up.which); break;
	case AW_EVENT_MOTION:
		Log("Motion %d: %d,%d", 
		    n, awe->u.motion.x, awe->u.motion.y); break;
	case AW_EVENT_CLOSE:
		Log("Exit requested");
		g_exit = 1; break;
	default: break;
	}
}

#define NWIN 6

static void draw(int n) {
	float f = (float)n / NWIN;
	glClearColor(0, 1 - f, f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void handle(aw * w, ac * c, int n) {
	awMakeCurrent(w, c);
	draw(n);
	awSwapBuffers(w);
	awMakeCurrent(w, 0);
	processEvents(w, c, n);
}

int main(int argc, char ** argv) {
	int i;
	aw * w[NWIN];
	ac * c[NWIN];
	ac * ic;
	awInit();
	ic = acNew(0);
	for (i = 0; i < NWIN; i++) {
		c[i] = acNew(ic);
		w[i] = awOpen(100 + 16*i, 100+16*i, 300, 400);
	}
	for (i = 0; i < NWIN; i++)
		awSetTitle(w[i], argv[0]);
	while (!g_exit) 
		for (i = 0; i < NWIN; i++) 
			handle(w[i], c[i], i);
	for (i = 0; i < NWIN; i++) 
		awClose(w[i]);
	awEnd();
	return 0;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
