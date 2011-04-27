#include <aw/sysgl.h>
#include <aw/aw.h>
#include "log.h"

static GLuint loadShader(GLenum type, const char * src)
{
	GLuint sh = glCreateShader(type);
	glShaderSource(sh, 1, &src, NULL);
	glCompileShader(sh);
	return sh;
}
static GLuint g_prg;
static GLint g_time;
static void init(void) 
{
	const char vss[] =
		"attribute vec4 pos;   \n"
		"uniform float t;            \n"
		"void main()                 \n"
		"{                           \n"
		"   gl_Position = pos; \n"
		"   gl_Position.y += sin(t) / 2.0; \n"
		"}                           \n";
	const char fss[] =
		"precision mediump float;                   \n"
		"void main()                                \n"
		"{                                          \n"
		" gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);  \n"
		"}                                          \n";
	GLuint vs, fs, prg;
	vs = loadShader(GL_VERTEX_SHADER, vss);
	fs = loadShader(GL_FRAGMENT_SHADER, fss);
	prg = glCreateProgram();
	glAttachShader(prg, vs);
	glAttachShader(prg, fs);
	glLinkProgram(prg);
	g_prg = prg;
	glBindAttribLocation(g_prg, 0, "vPosition");
	g_time = glGetUniformLocation(g_prg, "t");
}

static void display(void)
{
	GLfloat ver[] = {0.0f, 0.5f, 0.0f,
			 -0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f};
	static float t = 0;
	t += 1/60.0;
	glClearColor(0.7, 0.7, 0.7, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(g_prg);
	glUniform1f(g_time, t);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, ver);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

static void reshape (int w, int h)
{
	glViewport(0, 0, w, h);
}

#include "redbook.h"

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
