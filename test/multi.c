#include <aw/sysgl.h>
#include <aw/aw.h>
#include "log.h"

static int g_exit = 0;

static void processEvents(aw * w, int n) {
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
	}
}

#define NWIN 160

static void draw(int n) {
	float f = (float)n / NWIN;
	glClearColor(0, 1 - f, f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void handle(aw * w, int n) {
	awPushCurrent(w);
	draw(n);
	awSwapBuffers(w);
	processEvents(w, n);
	awPopCurrent(w);
}

int main(int argc, char ** argv) {
	int i;
	aw * w[NWIN];
	awInit();
	for (i = 0; i < NWIN; i++)
		w[i] = awOpen(argv[0], 100 + 16*i, 100+16*i, 300, 400);
	while (!g_exit) 
		for (i = 0; i < NWIN; i++) 
			handle(w[i], i);
	for (i = 0; i < NWIN; i++) 
		awClose(w[i]);
	awEnd();
	while (1)
		;
	return 0;
}

/* 
Local variables: **
c-file-style: "bsd" **
End: **
*/
