#define CURSOR_WIDTH 32
#define CURSOR_HEIGHT 32
#define CURSOR_BYTES (4*CURSOR_WIDTH*CURSOR_HEIGHT)
#define MAX_WINDOWS 256
#define MAX_EVENTS 1024
#define MAX_PRESSED 256

struct _ae {
	awcell type;
        awcell p[2];
};

typedef struct agHeader {
        int dummy;
} agHeader;

typedef struct awHeader {
        ac * ctx;
        ap * pointer;
        ag * g;
        void * user;
        ae * last;
        ae ev[MAX_EVENTS];
        unsigned interval;
        unsigned char pressed[MAX_PRESSED/8];
        unsigned char ppressed[MAX_PRESSED/8];
        unsigned head, tail;
        unsigned width, height;
        int mx, my;
        int fullscreen;
} awHeader;

typedef struct acHeader {
        ag * g;
} acHeader;

typedef struct apHeader {
        int refs;
} apHeader;

ag * agosNew(const char *);
int agosDel(ag *);
aw * awosOpen(ag *, int, int, int, int, int, int);
int awosSetTitle(aw *, const char *);
int awosClose(aw *);
int awosMakeCurrent(aw *, ac *);
int awosClearCurrent(aw *);
int awosSwapBuffers(aw *);
int awosShow(aw *);
int awosHide(aw *);
void awosPollEvent(aw *);
void awosThreadEvents();
int awosSetSwapInterval(aw *, int);
int awosGeometry(aw *, int, int, unsigned, unsigned);
void awosPointer(aw *);
unsigned awosOrder(aw **);

int acosInit();
int acosEnd();
ac * acosNew(ag *, ac *);
int acosDel(ac *);

ap * aposNew(const void * rgba, unsigned hotx, unsigned hoty);
int aposDel(ap *);

// Defined in the frontend
void got(aw  * w, int, intptr_t, intptr_t);
void report(const char * fmt, ...);
#if defined NDEBUG
static __inline void debug(const char * fmt, ...) {}
#else
#define debug report
#endif


#define wgroup(w) (w->hdr.g)
#define cgroup(c) (c->hdr.g)
/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
