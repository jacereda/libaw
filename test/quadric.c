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


static GLuint startList;

static void errorCallback(GLenum errorCode)
{
	Log("Quadric Error: %s", gluErrorString(errorCode));
}

static void init(void) 
{
	GLUquadricObj *qobj;
	GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

/*  Create 4 display lists, each with a different quadric object.
 *  Different drawing styles and surface normal specifications
 *  are demonstrated.
 */
	startList = glGenLists(4);
	qobj = gluNewQuadric();
	gluQuadricCallback(qobj, GLU_ERROR, 
			   errorCallback);

	gluQuadricDrawStyle(qobj, GLU_FILL); /* smooth shaded */
	gluQuadricNormals(qobj, GLU_SMOOTH);
	glNewList(startList, GL_COMPILE);
	gluSphere(qobj, 0.75, 15, 10);
	glEndList();

	gluQuadricDrawStyle(qobj, GLU_FILL); /* flat shaded */
	gluQuadricNormals(qobj, GLU_FLAT);
	glNewList(startList+1, GL_COMPILE);
	gluCylinder(qobj, 0.5, 0.3, 1.0, 15, 5);
	glEndList();

	gluQuadricDrawStyle(qobj, GLU_LINE); /* all polygons wireframe */
	gluQuadricNormals(qobj, GLU_NONE);
	glNewList(startList+2, GL_COMPILE);
	gluDisk(qobj, 0.25, 1.0, 20, 4);
	glEndList();

	gluQuadricDrawStyle(qobj, GLU_SILHOUETTE); /* boundary only  */
	gluQuadricNormals(qobj, GLU_NONE);
	glNewList(startList+3, GL_COMPILE);
	gluPartialDisk(qobj, 0.0, 1.0, 20, 4, 0.0, 225.0);
	glEndList();
}

static void display(void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glEnable(GL_LIGHTING);
	glShadeModel (GL_SMOOTH);
	glTranslatef(-1.0, -1.0, 0.0);
	glCallList(startList);

	glShadeModel (GL_FLAT);
	glTranslatef(0.0, 2.0, 0.0);
	glPushMatrix();
	glRotatef(300.0, 1.0, 0.0, 0.0);
	glCallList(startList+1);
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glColor3f(0.0, 1.0, 1.0);
	glTranslatef(2.0, -2.0, 0.0);
	glCallList(startList+2);

	glColor3f(1.0, 1.0, 0.0);
	glTranslatef(0.0, 2.0, 0.0);
	glCallList(startList+3);

	glPopMatrix();
	glFlush();
}

static void resize(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-2.5, 2.5, -2.5*(GLfloat)h/(GLfloat)w,
			2.5*(GLfloat)h/(GLfloat)w, -10.0, 10.0);
	else
		glOrtho(-2.5*(GLfloat)w/(GLfloat)h,
			2.5*(GLfloat)w/(GLfloat)h, -2.5, 2.5, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
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
