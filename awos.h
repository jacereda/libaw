#define CURSOR_WIDTH 32
#define CURSOR_HEIGHT 32
#define CURSOR_BYTES (4*CURSOR_WIDTH*CURSOR_HEIGHT)
#define MAX_WINDOWS 256
#define MAX_EVENTS 1024
#define MAX_PRESSED 256

#if defined _WIN32
#  include "aww.h"
#elif defined __ANDROID__
#  include "awandroid.h"
#elif defined __IPHONE_OS_VERSION_MIN_REQUIRED
#  include "awios.h"
#elif defined __APPLE__ && !defined BACKEND_x11
#  include "awcocoa.h"
#else
#  include "awx.h"
#endif

struct _ae {
	awcell type;
        awcell p[2];
};

#include "cotypes.h"
typedef struct _osw osw;

struct _aw {
        osw osw; // must be first
        ag * g;
        ac * ctx;
        ap * pointer;
        void * user;
        ae * last;
        ae ev[MAX_EVENTS];
        unsigned interval;
        unsigned char pressed[MAX_PRESSED/8];
        unsigned char ppressed[MAX_PRESSED/8];
        unsigned head, tail;
        unsigned width, height;
        int mx, my;
        int rx, ry, rw, rh;
        int maximized;
        int borders;
        int shown;
};

typedef struct _osg osg;

struct _ag {
        osg osg; // must be first
        const char * name;
};

typedef struct _osc osc;

struct _ac {
        osc osc; // must be first
        ag * g;
        int ok;
};

typedef struct _osp osp;

struct _ap {
        osp osp; // must be first
        int refs;
        int ok;
};

int osgInit(osg *, const char *);
int osgTerm(osg *);
void osgTick(osg *);

int oswInit(osw *, osg *, int, int, int, int, int);
int oswTerm(osw *);
int oswSetTitle(osw *, const char *);
int oswMakeCurrent(osw *, osc *);
int oswClearCurrent(osw *);
int oswSwapBuffers(osw *);
int oswShow(osw *);
int oswHide(osw *);
void oswPollEvent(osw *);
void oswThreadEvents();
int oswSetSwapInterval(osw *, int);
int oswMaximize(osw *);
int oswGeometry(osw *, int, int, unsigned, unsigned);
void oswPointer(osw *);
unsigned oswOrder(osw **);
int oswShowKeyboard(osw *);
int oswHideKeyboard(osw *);

int oscInit(osc *, osg *, osc *);
int oscTerm(osc *);

int ospInit(osp *, const void * rgba, unsigned hotx, unsigned hoty);
int ospTerm(osp *);

int osInit();
int osTerm();

// Defined in the frontend
void got(osw * w, int, intptr_t, intptr_t);
void report(const char * fmt, ...);
void progrun(int argc, char ** argv);
int progfinished();
int progterm();
void dispatch();
#if defined NDEBUG
static __inline void debug(const char * fmt, ...) {}
#else
#define debug report
#endif


#define wmaximized(w) (((aw*)w)->maximized)
#define wgroup(w) (&(((aw*)w)->g->osg))
#define cgroup(c) (&(((ac*)c)->g->osg))
#define wpointer(w) (&(((aw*)w)->pointer->osp))
#define wcontext(w) (&(((aw*)w)->ctx->osc))
#define wmousex(w) ((((aw*)w)->mx))
#define wmousey(w) ((((aw*)w)->my))
#define prgba(p) (((ap*)p)->rgba)

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
