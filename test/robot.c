/* Adapted from the Red Book */

#include <stdarg.h>
#include <stdio.h>
#include <aw/sysgl.h>
#include <aw/sysglu.h>
#include <aw/aw.h>
#include "log.h"

static int shoulder = 0, elbow = 0;

static void
drawBox(GLfloat size, GLenum type)
{
	static GLfloat n[6][3] =
		{
			{-1.0, 0.0, 0.0},
			{0.0, 1.0, 0.0},
			{1.0, 0.0, 0.0},
			{0.0, -1.0, 0.0},
			{0.0, 0.0, 1.0},
			{0.0, 0.0, -1.0}
		};
	static GLint faces[6][4] =
		{
			{0, 1, 2, 3},
			{3, 2, 6, 7},
			{7, 6, 5, 4},
			{4, 5, 1, 0},
			{5, 6, 2, 1},
			{7, 4, 0, 3}
		};
	GLfloat v[8][3];
	GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

	for (i = 5; i >= 0; i--) {
		glBegin(type);
		glNormal3fv(&n[i][0]);
		glVertex3fv(&v[faces[i][0]][0]);
		glVertex3fv(&v[faces[i][1]][0]);
		glVertex3fv(&v[faces[i][2]][0]);
		glVertex3fv(&v[faces[i][3]][0]);
		glEnd();
	}
}

static void init(void) 
{
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_FLAT);
}

static void display(void)
{
	glClear (GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	glTranslatef (-1.0, 0.0, 0.0);
	glRotatef ((GLfloat) shoulder, 0.0, 0.0, 1.0);
	glTranslatef (1.0, 0.0, 0.0);
	glPushMatrix();
	glScalef (2.0, 0.4, 1.0);
	drawBox(1.0, GL_LINE_LOOP);
	glPopMatrix();

	glTranslatef (1.0, 0.0, 0.0);
	glRotatef ((GLfloat) elbow, 0.0, 0.0, 1.0);
	glTranslatef (1.0, 0.0, 0.0);
	glPushMatrix();
	glScalef (2.0, 0.4, 1.0);
	drawBox(1.0, GL_LINE_LOOP);
	glPopMatrix();

	glPopMatrix();
}

static void reshape (int w, int h)
{
	glViewport (0, 0, w, h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(65.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef (0.0, 0.0, -5.0);
}

static void keyboard (unsigned char key)
{
	switch (key) {
	case 's':
		shoulder = (shoulder + 5) % 360;
		break;
	case 'd':
		shoulder = (shoulder - 5) % 360;
		break;
	case 'e':
		elbow = (elbow + 5) % 360;
		break;
	case 'r':
		elbow = (elbow - 5) % 360;
		break;
	default:
		break;
	}
}

static void updateTitle(aw * w, int x, int y);

static int processEvents(ag * g, aw * w, ac * c) {
	int keepgoing = 1;
	const ae * e;
	while ((e = awNextEvent(w))) switch (aeType(e)) {
	case AW_EVENT_RESIZE:
		reshape(aeWidth(e), aeHeight(e));
		updateTitle(w, aeWidth(e), aeHeight(e));
		break;
	case AW_EVENT_CLOSE:
		keepgoing = 0; 
		break;
	case AW_EVENT_DOWN:
	case AW_EVENT_UP:
		keyboard(aeWhich(e));
		break;
	}
	return keepgoing;
}

#define NO_HANDLER
#include "redbook.h"

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
