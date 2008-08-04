/* Adapted from the Red Book */

#include <stdarg.h>
#include <stdio.h>
#include <aw/sysgl.h>
#include <aw/sysglu.h>
#include <aw/aw.h>

static void Log(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	fflush(stdout);
	va_end(ap);
}

static void resize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

static int processEvents(aw * w) {
	int keepgoing = 1;
	const awEvent * awe;
	while ((awe = awCompressedNextEvent(w))) switch (awe->type) {
	case AW_EVENT_RESIZE:
		resize(awe->u.resize.w, awe->u.resize.h);
		break;
	case AW_EVENT_CLOSE:
		keepgoing = 0; 
		break;
	}
	return keepgoing;
}

static void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_POLYGON); {
		glVertex3f(0.25, 0.25, 0.0);
		glVertex3f(0.75, 0.25, 0.0);
		glVertex3f(0.75, 0.75, 0.0);
		glVertex3f(0.25, 0.75, 0.0);
	} glEnd();
	glFlush();
}

static void init(void) {
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

int main(int argc, char ** argv) {
	aw * w;
	awInit();
	w = awOpen();
	if (!w) {
		Log("unable to open window (is DISPLAY set?)");
		return 1;
	}
	awSetTitle(w, argv[0]);
	awResize(w, 500, 500);
	awShow(w);
	awPushCurrent(w);
	init();
	while (processEvents(w)) {
		display();
		awSwapBuffers(w);
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
