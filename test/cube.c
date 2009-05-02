/* Adapted from the Red Book */

#include <aw/sysgl.h>
#include <aw/sysglu.h>
#include <aw/aw.h>
#include "log.h"
#include "drawbox.h"

void init(void) 
{
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_FLAT);
}

void display(void)
{
	static int angle = 0;
	glClear (GL_COLOR_BUFFER_BIT);
	glColor3f (1.0, 1.0, 1.0);
	glLoadIdentity ();             /* clear the matrix */
	/* viewing transformation  */
	gluLookAt (0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glScalef (1.0, 2.0, 1.0);      /* modeling transformation */ 
	glRotatef(angle++, 0,0,1);
	drawBox(1.0, GL_LINE_LOOP);
	glFlush ();
}

static void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
	glMatrixMode (GL_MODELVIEW);
}

static void updateTitle(aw * w, int x, int y) {
	char buf[16] = {0};
	snprintf(buf, sizeof(buf), "%dx%d", x, y);
	awSetTitle(w, buf);
}

static int processEvents(aw * w) {
	int keepgoing = 1;
	const awEvent * awe;
	while ((awe = awNextEvent(w))) switch (awe->type) {
	case AW_EVENT_RESIZE:
		reshape(awe->u.resize.w, awe->u.resize.h);
		updateTitle(w, awe->u.resize.w, awe->u.resize.h);
		break;
	case AW_EVENT_CLOSE:
		keepgoing = 0; 
		break;
	}
	return keepgoing;
}

#include "redbook.h"

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
