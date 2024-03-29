/* Adapted from the Red Book */

#include <stdarg.h>
#include <stdio.h>
#include <aw/sysgl.h>
#include <aw/sysglu.h>
#include <aw/aw.h>
#include "log.h"
static int board[3][3];   /*  amount of color for each square*/

/*  Clear color value for every square on the board   */
static void init(void)
{
	int i, j;
	for (i = 0; i < 3; i++) 
		for (j = 0; j < 3; j ++)
			board[i][j] = 0;
	glClearColor (0.0, 0.0, 0.0, 0.0);
}

/*  The nine squares are drawn.  In selection mode, each 
 *  square is given two names:  one for the row and the 
 *  other for the column on the grid.  The color of each 
 *  square is determined by its position on the grid, and 
 *  the value in the board[][] array.
 */
static void drawSquares(GLenum mode)
{
	GLuint i, j;
	for (i = 0; i < 3; i++) {
		if (mode == GL_SELECT)
			glLoadName (i);
		for (j = 0; j < 3; j ++) {
			if (mode == GL_SELECT)
				glPushName (j);
			glColor3f ((GLfloat) i/3.0, (GLfloat) j/3.0, 
				   (GLfloat) board[i][j]/3.0);
			glRecti (i, j, i+1, j+1);
			if (mode == GL_SELECT)
				glPopName ();
		}
	}
}

/*  processHits prints out the contents of the 
 *  selection array.
 */
static void processHits (GLint hits, GLuint buffer[])
{
	unsigned int i, j;
	GLuint ii=0, jj=0, names, *ptr;

	Log("hits = %d", (int)hits);
	ptr = (GLuint *) buffer;
	for (i = 0; i < hits; i++) {/*  for each hit  */
		names = *ptr;
		Log(" number of names for this hit = %d", names); ptr++;
		Log("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
		Log("  z2 is %g", (float) *ptr/0x7fffffff); ptr++;
		Log("  names are: ");
		for (j = 0; j < names; j++) { /*  for each name */
			Log("    %d ", *ptr);
			if (j == 0)  /*  set row and column  */
				ii = *ptr;
			else if (j == 1)
				jj = *ptr;
			ptr++;
		}
		board[ii][jj] = (board[ii][jj] + 1) % 3;
	}
}

/*  pickSquares() sets up selection mode, name stack, 
 *  and projection matrix for picking.  Then the 
 *  objects are drawn.
 */
#define BUFSIZE 512

static void pickSquares(int x, int y)
{
	GLuint selectBuf[BUFSIZE];
	GLint hits;
	GLint viewport[4];

	glGetIntegerv (GL_VIEWPORT, viewport);

	glSelectBuffer (BUFSIZE, selectBuf);
	(void) glRenderMode (GL_SELECT);

	glInitNames();
	glPushName(0);

	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	/*  create 5x5 pixel picking region near cursor location*/
	gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 
		       5.0, 5.0, viewport);
	gluOrtho2D (0.0, 3.0, 0.0, 3.0);
	drawSquares (GL_SELECT);

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glFlush ();

	hits = glRenderMode (GL_RENDER);
	processHits (hits, selectBuf);
} 

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawSquares (GL_RENDER);
	glFlush();
}

static void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D (0.0, 3.0, 0.0, 3.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void updateTitle(aw * w, int x, int y);

static int processEvents(ag * g, aw * w, ac * c) {
	int keepgoing = 1;
	static int s_x = 0;
	static int s_y = 0;
	const ae * e;
	while ((e = awNextEvent(w))) switch (aeType(e)) {
	case AW_EVENT_RESIZE:
		reshape(aeWidth(e), aeHeight(e));
		updateTitle(w, aeWidth(e), aeHeight(e));
		break;
	case AW_EVENT_CLOSE:
		keepgoing = 0; 
		break;
	case AW_EVENT_MOTION:
		s_x = aeX(e);
		s_y = aeY(e);
		break;
	case AW_EVENT_DOWN:
		if (aeWhich(e) == AW_KEY_MOUSELEFT)
			pickSquares(s_x, s_y);
		break;
	default: break;
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
