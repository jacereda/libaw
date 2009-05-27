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
	glScalef (1.0, 1.0, 1.0);      /* modeling transformation */ 
	glRotatef(angle++, 0,0,1);
	drawBox(1.0, GL_LINE_LOOP);
	glFlush ();
}

static void reshape (int w, int h)
{
	float aspect = (float)w / h;
	glViewport (0, 0, w, h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glFrustum (-aspect, aspect, -1.0, 1.0, 1.5, 20.0);
	glMatrixMode (GL_MODELVIEW);
}

#include "redbook.h"

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
