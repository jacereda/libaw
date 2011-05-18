#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include "aw.h"
#include "awos.h"

#if !defined HANDLE_WM_MOUSEWHEEL
#define HANDLE_WM_MOUSEWHEEL(hwnd,wParam,lParam,fn) ((fn)((hwnd),(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam),(int)(short)HIWORD(wParam),(UINT)(short)LOWORD(wParam)),0L)
#endif

osw * oswFor(HWND win) {
        return (osw*)GetWindowLongPtrW(win, GWLP_USERDATA);
}

unsigned oswOrder(osw ** wins) {
        HWND win = GetTopWindow(0);
        unsigned n = 0;
        int curr = GetCurrentProcessId();
        while (win) {
                osw * w = oswFor(win);
                DWORD pid;
                GetWindowThreadProcessId(win, &pid);
                if (curr == pid && w)
                        wins[n++] = w;
                win = GetNextWindow(win, GW_HWNDNEXT);
        };
        return n;
}

static int wgot(HWND win, int type, intptr_t p1, intptr_t p2) {
        osw * w = oswFor(win);
        if (w)
                got(w, type, p1, p2);
        return w != 0;
}

static int onMOUSEMOVE(HWND win, int x, int y, UINT flags ) {
        return wgot(win, AW_EVENT_MOTION, x, y);
}

static int onMOVE(HWND win, int x, int y) {
        return wgot(win, AW_EVENT_POSITION, x, y);
}

static int onSIZE(HWND win, UINT state, int w, int h) {
        return wgot(win, AW_EVENT_RESIZE, w, h);
}

static int onCLOSE(HWND win){
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

static int onSYSKEYDOWN(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        return wgot(win, AW_EVENT_DOWN, mapkeycode(vk), 0);
}

static int onSYSKEYUP(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        return wgot(win, AW_EVENT_UP, mapkeycode(vk), 0);
}

static int onKEYDOWN(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        return wgot(win, AW_EVENT_DOWN, mapkeycode(vk), 0);
}

static int onKEYUP(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
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

static int onLBUTTONDOWN(HWND win, BOOL dbl, int x, int y, UINT flags) {
        return mouseDown(win, AW_KEY_MOUSELEFT);
}

static int onMBUTTONDOWN(HWND win, BOOL dbl, int x, int y, UINT flags) {
        return mouseDown(win, AW_KEY_MOUSEMIDDLE);
}

static int onRBUTTONDOWN(HWND win, BOOL dbl, int x, int y, UINT flags) {
        return mouseDown(win, AW_KEY_MOUSERIGHT);
}

static int onLBUTTONUP(HWND win, int x, int y, UINT flags) {
        return mouseUp(win, AW_KEY_MOUSELEFT);
}

static int onMBUTTONUP(HWND win, int x, int y, UINT flags) {
        return mouseUp(win, AW_KEY_MOUSEMIDDLE);
}

static int onRBUTTONUP(HWND win, int x, int y, UINT flags) {
        return mouseUp(win, AW_KEY_MOUSERIGHT);
}

static int onMOUSEWHEEL(HWND win, int x, int y, int z, UINT keys) {
        int which = z >= 0? AW_KEY_MOUSEWHEELUP : AW_KEY_MOUSEWHEELDOWN;
        return wgot(win, AW_EVENT_DOWN, which, 0)
                && wgot(win, AW_EVENT_UP, which, 0);
}

static int onSETCURSOR(HWND win, HWND cur, UINT l, UINT h) {
        osw * w = oswFor(win);
        int handled = LOWORD(l) == HTCLIENT && w != 0;
        extern void oswSetPointer(osw *);
        if (handled)
                oswSetPointer(w);
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

static int onSETFOCUS(HWND win, HWND next) {
        wgot(win, AW_EVENT_SETFOCUS, (intptr_t)oswFor(next), 0);
        return fakeKbEvents(win, AW_EVENT_DOWN);
}

static int onKILLFOCUS(HWND win, HWND next) {
        wgot(win, AW_EVENT_KILLFOCUS, (intptr_t)oswFor(next), 0);
        return fakeKbEvents(win, AW_EVENT_UP);
}

static int onCHAR(HWND win, TCHAR c, int repeats) {
        return wgot(win, AW_EVENT_UNICODE, uc2aw(c), 0); 
}

static int onPAINT(HWND win) {
        return wgot(win, AW_EVENT_EXPOSED, 0, 0);
}

static void utf8(char * d, size_t dsz, const WCHAR * t) {
        WideCharToMultiByte(CP_UTF8, 0, t, -1, d, (int)dsz, 0, 0);
}

static int onDROPFILES(HWND win, HDROP d) {
        unsigned n = DragQueryFileW(d, 0xffffffff, 0, 0);
        unsigned i;
        int ret;
        for (i = 0; i < n; i++) {
                WCHAR buf[8192];
                char str[8192];
                DragQueryFileW(d, i, buf, sizeof(buf));
                utf8(str, sizeof(str), buf);
                ret = wgot(win, AW_EVENT_DROP, 
                           (intptr_t)_strdup(str), 0);
        }
        DragFinish(d);
        return ret;
}

static int onNCCREATE(HWND win, CREATESTRUCT * cs) {
        SetWindowLongPtrW(win, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        return 0;
}


LRESULT WINAPI handle(HWND win, UINT msg, WPARAM w, LPARAM l)  {
        LRESULT r;
        switch (msg) {
#define HANDLE(x) case WM_##x: r = HANDLE_WM_##x(win, w, l, on##x); break
                HANDLE(NCCREATE);
                HANDLE(MOUSEMOVE);
                HANDLE(MOVE);
                HANDLE(SIZE);
                HANDLE(CLOSE);
                HANDLE(KEYDOWN);
                HANDLE(SYSKEYDOWN);
                HANDLE(SYSKEYUP);
                HANDLE(CHAR);
                HANDLE(KEYUP);
                HANDLE(LBUTTONDOWN);
                HANDLE(RBUTTONDOWN);
                HANDLE(MBUTTONDOWN);
                HANDLE(LBUTTONUP);
                HANDLE(RBUTTONUP);
                HANDLE(MBUTTONUP);
                HANDLE(MOUSEWHEEL);
                HANDLE(SETFOCUS);
                HANDLE(KILLFOCUS);
                HANDLE(SETCURSOR);
                HANDLE(PAINT);
                HANDLE(DROPFILES);
#undef HANDLE
        case WM_IME_STARTCOMPOSITION: 
        {
                osw * w = oswFor(win);
                if (w) {
                        HIMC imc = ImmGetContext(win);
                        COMPOSITIONFORM cf;
                        cf.dwStyle = CFS_POINT;
                        cf.ptCurrentPos.x = wmousex(w);
                        cf.ptCurrentPos.y = wmousey(w);
                        ImmSetCompositionWindow(imc, &cf);
                        ImmReleaseContext(win, imc);
                }
                r = w != 0;
        }
        break;
        case WM_IME_COMPOSITION: 
        {
                osw * w = oswFor(win);
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
