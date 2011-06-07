#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>

struct _osg {
        HANDLE ready;
        HANDLE thread;
        HWND win;
        WCHAR appname[256];
};

struct _osw {
        HWND win;
        DWORD style;
};

struct _osc {
        HGLRC ctx;
};

struct _osp {
        HCURSOR icon;
        ICONINFO ii;
};
