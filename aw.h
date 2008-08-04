enum {
  AW_EVENT_UNKNOWN,
  AW_EVENT_NONE,
  AW_EVENT_MOVE,
  AW_EVENT_RESIZE,
  AW_EVENT_CLOSE,
  AW_EVENT_DOWN,
  AW_EVENT_UP,
  AW_EVENT_MOTION,
};

enum {
  AW_KEY_NONE,
  AW_KEY_MOUSEWHEELUP, 
  AW_KEY_MOUSEWHEELDOWN,
  AW_KEY_MOUSELEFT, 
  AW_KEY_MOUSEMIDDLE,
  AW_KEY_MOUSERIGHT,
};

typedef struct awEvent {
  int type;
  union {
    int p[2];
    struct { int x, y; } move;
    struct { int w, h; } resize;
    struct { int which; } down;
    struct { int which; } up;
    struct { int x, y; } motion;
  } u;
} awEvent;

typedef struct _aw aw;

#if defined(__GNUC__)
#define EXPORTED extern __attribute__((visibility("default")))
#else
#define EXPORTED extern __declspec(dllexport)
#endif

EXPORTED int awInit();
EXPORTED void awEnd();
EXPORTED aw * awOpen();
EXPORTED aw * awOpenSharing(void *);
EXPORTED void awClose(aw *);
EXPORTED void awSwapBuffers(aw *);
EXPORTED void awPushCurrent(aw *);
EXPORTED void awPopCurrent(aw *);
EXPORTED void awShow(aw *);
EXPORTED void awHide(aw *);
EXPORTED void awSetTitle(aw *, const char *);
EXPORTED void awMove(aw *, int, int);
EXPORTED void awResize(aw *, int, int);
EXPORTED const awEvent * awNextEvent(aw *);
EXPORTED const awEvent * awCompressedNextEvent(aw *);
