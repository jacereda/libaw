#if !defined __OBJC__
typedef struct _View View;
typedef struct _Window Window;
typedef struct _NSCursor NSCursor;
typedef struct _NSOpenGLContext NSOpenGLContext;
#endif

struct _osg {
	int dummy;
};

struct _osw {
        View * view;
        Window * win;
        NSCursor * defcur;
        int _vfreed;
        int _wfreed;
};

struct _osc {
        NSOpenGLContext * ctx;
};

struct _osp {
        NSCursor * cur;
        uint8_t rgba[CURSOR_BYTES];
};

