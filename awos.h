
#define MAX_EVENTS 128

typedef struct awHeader {
	int interval;
	awEvent ev[MAX_EVENTS];
	unsigned head, tail;
	} awHeader;

int awosInit();
void awosEnd();
aw * awosOpen(const char *, int, int, int, int, void *);
int awosClose(aw *);
int awosSwapBuffers(aw *);
int awosPushCurrent(aw *);
int awosPopCurrent(aw *);
int awosShow(aw *);
int awosHide(aw *);
int awosSetTitle(aw *, const char *);
int awosResize(aw *, int, int);
void awosPollEvent(aw *);
int awosSetSwapInterval(int);

// Defined in the frontend
void got(aw  * w, int, int, int);
void report(const char * fmt, ...);
