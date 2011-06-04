#include <X11/Xlib.h>
#include <GL/glx.h>

struct _osg {
        Display * dpy;
        int screen;
        XIM xim;
        Atom del;
        int(*swapInterval)(int);
        int bw, bh;
};

struct _osw {
        Window win;
        int lastw, lasth;
        int lastx, lasty;
        XIC xic;
};

struct _osc {
        GLXContext ctx;
};

struct _osp {
	int dummy;
};
