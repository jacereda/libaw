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

void init(void) 
{
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_FLAT);
}

void display(void)
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

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(65.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef (0.0, 0.0, -5.0);
}

void keyboard (unsigned char key)
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

static int processEvents(aw * w) {
	int keepgoing = 1;
	const awEvent * awe;
	while ((awe = awCompressedNextEvent(w))) switch (awe->type) {
	case AW_EVENT_RESIZE:
		reshape(awe->u.resize.w, awe->u.resize.h);
		break;
	case AW_EVENT_CLOSE:
		keepgoing = 0; 
		break;
	case AW_EVENT_DOWN:
	case AW_EVENT_UP:
		keyboard(awe->u.up.which);
		break;
	}
	return keepgoing;
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
	Log("Keys:\n"
	    "s - increase shoulder angle\n"
	    "d - decrease shoulder angle\n"
	    "e - increase elbow angle\n"
	    "r - decrease elbow angle\n");
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
