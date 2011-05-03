#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include "aw.h"
#include "awos.h"

#if !defined HANDLE_WM_MOUSEWHEEL
#define HANDLE_WM_MOUSEWHEEL(hwnd,wParam,lParam,fn) ((fn)((hwnd),(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam),(int)(short)HIWORD(wParam),(UINT)(short)LOWORD(wParam)),0L)
#endif

aw * awFor(HWND win) {
        return (aw*)GetWindowLongPtrW(win, GWLP_USERDATA);
}

unsigned awosOrder(aw ** wins) {
        HWND win = GetTopWindow(0);
        unsigned n = 0;
        int curr = GetCurrentProcessId();
        while (win) {
                aw * w = awFor(win);
                DWORD pid;
                GetWindowThreadProcessId(win, &pid);
                if (curr == pid && w)
                        wins[n++] = w;
                win = GetNextWindow(win, GW_HWNDNEXT);
        };
        return n;
}

static int wgot(HWND win, int type, intptr_t p1, intptr_t p2) {
        aw * w = awFor(win);
        if (w)
                got(w, type, p1, p2);
        return w != 0;
}

static int onMMove(HWND win, int x, int y, UINT flags ) {
        return wgot(win, AW_EVENT_MOTION, x, y);
}

static int onMove(HWND win, int x, int y) {
        return wgot(win, AW_EVENT_POSITION, x, y);
}

static int onSize(HWND win, UINT state, int w, int h) {
        return wgot(win, AW_EVENT_RESIZE, w, h);
}

static int onClose(HWND win){
        return wgot(win, AW_EVENT_CLOSE, 0, 0);
}

static unsigned uc2aw(unsigned uc) {
        unsigned ret = uc;
        switch (uc) {
        case 0xd: ret = '\n'; break;
        }
        return ret;
}

extern unsigned mapkeycode(unsigned);

static int onSysKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        return wgot(win, AW_EVENT_DOWN, mapkeycode(vk), 0);
}

static int onSysKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        return wgot(win, AW_EVENT_UP, mapkeycode(vk), 0);
}

static int onKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        return wgot(win, AW_EVENT_DOWN, mapkeycode(vk), 0);
}

static int onKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        return wgot(win, AW_EVENT_UP, mapkeycode(vk), 0);
}

static int mouseDown(HWND win, int which) {
        SetCapture(win);
        return wgot(win, AW_EVENT_DOWN, which, 0);        
}

static int mouseUp(HWND win, int which) {
        int ret = wgot(win, AW_EVENT_UP, which, 0);        
        ReleaseCapture();
        return ret;
}

static int onLD(HWND win, BOOL dbl, int x, int y, UINT flags) {
        return mouseDown(win, AW_KEY_MOUSELEFT);
}

static int onMD(HWND win, BOOL dbl, int x, int y, UINT flags) {
        return mouseDown(win, AW_KEY_MOUSEMIDDLE);
}

static int onRD(HWND win, BOOL dbl, int x, int y, UINT flags) {
        return mouseDown(win, AW_KEY_MOUSERIGHT);
}

static int onLU(HWND win, int x, int y, UINT flags) {
        return mouseUp(win, AW_KEY_MOUSELEFT);
}

static int onMU(HWND win, int x, int y, UINT flags) {
        return mouseUp(win, AW_KEY_MOUSEMIDDLE);
}

static int onRU(HWND win, int x, int y, UINT flags) {
        return mouseUp(win, AW_KEY_MOUSERIGHT);
}

static int onMW(HWND win, int x, int y, int z, UINT keys) {
        int which = z >= 0? AW_KEY_MOUSEWHEELUP : AW_KEY_MOUSEWHEELDOWN;
        return wgot(win, AW_EVENT_DOWN, which, 0)
                && wgot(win, AW_EVENT_UP, which, 0);
}

static int onSetCursor(HWND win, HWND cur, UINT l, UINT h) {
        aw * w = awFor(win);
        int handled = LOWORD(l) == HTCLIENT && w != 0;
        extern void awosSetPointer(aw *);
        if (handled)
                awosSetPointer(w);
        return handled;
}

static int fakeKbEvents(HWND win, unsigned type) {
        BYTE kbstatus[256];
        UINT vk;
        GetKeyboardState(kbstatus);
        for (vk = 0; vk < sizeof(kbstatus); vk++)
                if (kbstatus[vk] & 0x80)
                        wgot(win, type, mapkeycode(vk), 0);
        return 0;
}

static int onSetFocus(HWND win, HWND next) {
        wgot(win, AW_EVENT_SETFOCUS, 0, 0);
        return fakeKbEvents(win, AW_EVENT_DOWN);
}

static int onKillFocus(HWND win, HWND next) {
        wgot(win, AW_EVENT_KILLFOCUS, (intptr_t)awFor(next), 0);
        return fakeKbEvents(win, AW_EVENT_UP);
}

static int onChar(HWND win, TCHAR c, int repeats) {
        return wgot(win, AW_EVENT_UNICODE, uc2aw(c), 0); 
}

static int onPaint(HWND win) {
        return wgot(win, AW_EVENT_EXPOSED, 0, 0);
}

static void utf8(char * d, size_t dsz, const WCHAR * t) {
        WideCharToMultiByte(CP_UTF8, 0, t, -1, d, (int)dsz, 0, 0);
}

static int onDrop(HWND win, HDROP d) {
        unsigned n = DragQueryFile(d, 0xffffffff, 0, 0);
        unsigned i;
        int ret;
        for (i = 0; i < n; i++) {
                WCHAR buf[8192];
                char str[8192];
                DragQueryFile(d, i, buf, sizeof(buf));
                utf8(str, sizeof(str), buf);
                ret = wgot(win, AW_EVENT_DROP, 
                           (intptr_t)_strdup(str), 0);
        }
        DragFinish(d);
        return ret;
}

static int onNCCreate(HWND win, CREATESTRUCT * cs) {
        SetWindowLongPtrW(win, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        return 0;
}

LRESULT WINAPI handle(HWND win, UINT msg, WPARAM w, LPARAM l)  {
        LRESULT r;
        switch (msg) {
        case WM_NCCREATE: r = HANDLE_WM_NCCREATE(win, w, l, onNCCreate); break;
        case WM_MOUSEMOVE: r = HANDLE_WM_MOUSEMOVE(win, w, l, onMMove); break;
        case WM_MOVE: r = HANDLE_WM_MOVE(win, w, l, onMove); break;
        case WM_SIZE: r = HANDLE_WM_SIZE(win, w, l, onSize); break;
        case WM_CLOSE: r = HANDLE_WM_CLOSE(win, w, l, onClose); break;
        case WM_KEYDOWN: r = HANDLE_WM_KEYDOWN(win, w, l, onKeyDown); break;
        case WM_SYSKEYDOWN: r = HANDLE_WM_SYSKEYDOWN(win, w, l, onSysKeyDown); break;
        case WM_SYSKEYUP: r = HANDLE_WM_SYSKEYUP(win, w, l, onSysKeyUp); break;
        case WM_CHAR: r = HANDLE_WM_CHAR(win, w, l, onChar); break;
        case WM_KEYUP: r = HANDLE_WM_KEYUP(win, w, l, onKeyUp); break;
        case WM_LBUTTONDOWN: r = HANDLE_WM_LBUTTONDOWN(win, w, l, onLD); break;
        case WM_RBUTTONDOWN: r = HANDLE_WM_RBUTTONDOWN(win, w, l, onRD); break;
        case WM_MBUTTONDOWN: r = HANDLE_WM_MBUTTONDOWN(win, w, l, onMD); break;
        case WM_LBUTTONUP: r = HANDLE_WM_LBUTTONUP(win, w, l, onLU); break;
        case WM_RBUTTONUP: r = HANDLE_WM_RBUTTONUP(win, w, l, onRU); break;
        case WM_MBUTTONUP: r = HANDLE_WM_MBUTTONUP(win, w, l, onMU); break;
        case WM_MOUSEWHEEL: r = HANDLE_WM_MOUSEWHEEL(win, w, l, onMW); break;
        case WM_SETFOCUS: r = HANDLE_WM_SETFOCUS(win, w, l, onSetFocus); break;
        case WM_KILLFOCUS: r = HANDLE_WM_KILLFOCUS(win, w, l, onKillFocus); break;
        case WM_SETCURSOR: r = HANDLE_WM_SETCURSOR(win, w, l, onSetCursor); break;
        case WM_PAINT: r = HANDLE_WM_PAINT(win, w, l, onPaint); break;
        case WM_DROPFILES: r = HANDLE_WM_DROPFILES(win, w, l, onDrop); break;
        case WM_IME_STARTCOMPOSITION: 
        {
                awHeader * w = (awHeader*)awFor(win);
                if (w) {
                        HIMC imc = ImmGetContext(win);
                        COMPOSITIONFORM cf;
                        cf.dwStyle = CFS_POINT;
                        cf.ptCurrentPos.x = w->mx;
                        cf.ptCurrentPos.y = w->my;
                        ImmSetCompositionWindow(imc, &cf);
                        ImmReleaseContext(win, imc);
                }
                r = w != 0;
        }
        break;
        case WM_IME_COMPOSITION: 
        {
                awHeader * w = (awHeader*)awFor(win);
                if(w && (l & GCS_RESULTSTR)){
                        unsigned short str[4096];
                        unsigned len, i; 
                        HIMC imc = ImmGetContext(win);
                        HDC dc = GetDC(win);
                        len = ImmGetCompositionString(imc, GCS_RESULTSTR, str, sizeof(str));
                        len >>= 1;
                        for (i = 0; i < len; i++)
                                wgot(win, AW_EVENT_UNICODE, str[i], 0); 
                        ImmReleaseContext(win, imc);
                        ReleaseDC(win, dc);
                }
                r = 0;
        }
        break;
        default: r = 0;
        }
        if (!r)
                r = DefWindowProcW(win, msg, w, l);
        return r;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
