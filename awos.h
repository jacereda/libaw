
#define MAX_EVENTS 16

#define MAX_PRESSED 256

typedef struct awHeader {
	ac * ctx;
	unsigned head, tail;
	unsigned width, height;
	int mx, my;
	int fullscreen;
	unsigned char pressed[MAX_PRESSED/8];
	awEvent ev[MAX_EVENTS];
} awHeader;

typedef struct acHeader {
	int interval;
} acHeader;

int awosInit();
int awosEnd();
aw * awosOpen(int, int, int, int, int, int);
int awosSetTitle(aw *, const char *);
int awosClose(aw *);
int awosMakeCurrent(aw *, ac *);
int awosClearCurrent(aw *);
int awosSwapBuffers(aw *);
int awosShow(aw *);
int awosHide(aw *);
void awosPollEvent(aw *);
int awosSetSwapInterval(aw *, int);

int acosInit();
int acosEnd();
ac * acosNew(ac *);
int acosDel(ac *);

// Defined in the frontend
void got(aw  * w, int, int, int);
void report(const char * fmt, ...);

/* 
   Local variables: **
   c-file-style: "bsd" **
   End: **
*/
