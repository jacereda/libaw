#include <aw/sysgl.h>
#include <aw/aw.h>
#include "log.h"

static int g_exit = 0;

static void processEvents(aw * w, ac * c, int n) {
	const ae * e;
	while ((e = awNextEvent(w))) switch (aeType(e)) {
		case AW_EVENT_RESIZE:
			Log("Resized %d: %d %d", 
			    n, aeWidth(e), aeHeight(e)); break;
		case AW_EVENT_DOWN:
			Log("Down %d: %d", n, aeWhich(e)); break;
		case AW_EVENT_UP:
			Log("Up %d: %d", n, aeWhich(e)); break;
		case AW_EVENT_MOTION:
			Log("Motion %d: %d,%d", 
			    n, aeX(e), aeY(e)); break;
		case AW_EVENT_CLOSE:
			Log("Exit requested");
			g_exit = 1; break;
		}
}

#define NWIN 4

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
		ac * c;
		ag * g;
		g = agNew("multi");
		c = acNew(g, 0);
		for (i = 0; i < NWIN; i++)
				w[i] = awNew(g);
		for (i = 0; i < NWIN; i++)
				awSetInterval(w[i], 1);
		for (i = 0; i < NWIN; i++)
				awGeometry(w[i], 100 + 16*i, 100+16*i, 300, 400);
		for (i = 0; i < NWIN; i++)
				awSetTitle(w[i], argv[0]);
		for (i = 0; i < NWIN; i++)
				awShow(w[i]);
		while (!g_exit) 
				for (i = 0; i < NWIN; i++) {
						Log("Order %d %d", i, awOrder(w[i]));
						handle(w[i], c, i);
				}
		for (i = 0; i < NWIN; i++) 
				awDel(w[i]);
		agDel(g);
		return 0;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
