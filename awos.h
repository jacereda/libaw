
#define MAX_EVENTS 16

typedef struct awHeader {
	int interval;
	awEvent ev[MAX_EVENTS];
	unsigned head, tail;
	void * pushctx;
	void * pushdrw;
	} awHeader;

int awosInit();
void awosEnd();
aw * awosOpen(const char *, int, int, int, int, void *);
int awosClose(aw *);
void * awosGetCurrentContext();
void * awosGetCurrentDrawable();
void * awosGetContext(aw *);
void * awosGetDrawable(aw *);
int awosMakeCurrent(void *, void *);
int awosSwapBuffers(aw *);
int awosShow(aw *);
int awosHide(aw *);
int awosSetTitle(aw *, const char *);
int awosResize(aw *, int, int);
void awosPollEvent(aw *);
int awosSetSwapInterval(int);

// Defined in the frontend
void got(aw  * w, int, int, int);
void report(const char * fmt, ...);

/* 
   Local variables: **
   c-file-style: "bsd" **
   End: **
*/
