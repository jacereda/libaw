typedef struct awHeader {
	awEvent curr;
	awEvent next;
	int interval;
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
void awosNextEvent(aw *);
int awosSetSwapInterval(int);
