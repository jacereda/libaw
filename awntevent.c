#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include "aw.h"
#include "awos.h"

static aw * awFor(HWND win) {
        return (aw*)GetWindowLongPtrW(win, GWL_USERDATA);
}

static void wgot(HWND win, int type, int p1, int p2) {
        aw * w = awFor(win);
        if (w)
                got(w, type, p1, p2);
}

static void onMMove(HWND win, int x, int y, UINT flags ) {
        aw * w = awFor(win);
//        if (x != w->lastmx || y != w->lastmy)
                got(w, AW_EVENT_MOTION, x, y);
//        w->lastmx = x; w->lastmy = y;
}

static void onSize(HWND win, UINT state, int w, int h) {
        wgot(win, AW_EVENT_RESIZE, w, h);
}

static void onClose(HWND win){
        wgot(win, AW_EVENT_CLOSE, 0, 0);
}

static unsigned uc2aw(unsigned uc) {
        unsigned ret = uc;
        switch (uc) {
        case 0xd: ret = '\n'; break;
        }
        return ret;
}

extern awkey mapkey(unsigned);

static void onSysKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        if (vk == VK_MENU)
                wgot(win, AW_EVENT_DOWN, mapkey(vk), 0);
}

static void onSysKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        if (vk == VK_MENU)
                wgot(win, AW_EVENT_UP, mapkey(vk), 0);
}

static void onKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        wgot(win, AW_EVENT_DOWN, mapkey(vk), 0);
}

static void onKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        wgot(win, AW_EVENT_UP, mapkey(vk), 0);
}

static void onLD(HWND win, BOOL dbl, int x, int y, UINT flags) {
        wgot(win, AW_EVENT_DOWN, AW_KEY_MOUSELEFT, 0);
}

static void onMD(HWND win, BOOL dbl, int x, int y, UINT flags) {
        wgot(win, AW_EVENT_DOWN, AW_KEY_MOUSEMIDDLE, 0);
}

static void onRD(HWND win, BOOL dbl, int x, int y, UINT flags) {
        wgot(win, AW_EVENT_DOWN, AW_KEY_MOUSERIGHT, 0);
}

static void onLU(HWND win, int x, int y, UINT flags) {
        wgot(win, AW_EVENT_UP, AW_KEY_MOUSELEFT, 0);
}

static void onMU(HWND win, int x, int y, UINT flags) {
        wgot(win, AW_EVENT_UP, AW_KEY_MOUSEMIDDLE, 0);
}

static void onRU(HWND win, int x, int y, UINT flags) {
        wgot(win, AW_EVENT_UP, AW_KEY_MOUSERIGHT, 0);
}

static void onMW(HWND win, int x, int y, int z, UINT keys) {
        int which = z >= 0? AW_KEY_MOUSEWHEELUP : AW_KEY_MOUSEWHEELDOWN;
        wgot(win, AW_EVENT_DOWN, which, 0);
        wgot(win, AW_EVENT_UP, which, 0);
}


LONG WINAPI handle(HWND win, UINT msg, WPARAM w, LPARAM l)  {
        LONG r;
        int handled = 1;
        switch (msg) {
        case WM_NCCREATE: 
                SetWindowLongPtrW(
                        win, GWL_USERDATA, 
                        (LONG_PTR)((CREATESTRUCT*)l)->lpCreateParams);
                handled = 0;
                break;
        case WM_MOUSEMOVE: r = HANDLE_WM_MOUSEMOVE(win, w, l, onMMove); break;
        case WM_SIZE: r = HANDLE_WM_SIZE(win, w, l, onSize); break;
        case WM_CLOSE: r = HANDLE_WM_CLOSE(win, w, l, onClose); break;
        case WM_KEYDOWN: r = HANDLE_WM_KEYDOWN(win, w, l, onKeyDown); break;
        case WM_SYSKEYDOWN: r = HANDLE_WM_SYSKEYDOWN(win, w, l, onSysKeyDown); break;
        case WM_SYSKEYUP: r = HANDLE_WM_SYSKEYUP(win, w, l, onSysKeyUp); break;
        case WM_CHAR: wgot(win, AW_EVENT_UNICODE, uc2aw(w), 0); break;
        case WM_KEYUP: r = HANDLE_WM_KEYUP(win, w, l, onKeyUp); break;
        case WM_LBUTTONDOWN: r = HANDLE_WM_LBUTTONDOWN(win, w, l, onLD); break;
        case WM_RBUTTONDOWN: r = HANDLE_WM_RBUTTONDOWN(win, w, l, onRD); break;
        case WM_MBUTTONDOWN: r = HANDLE_WM_MBUTTONDOWN(win, w, l, onMD); break;
        case WM_LBUTTONUP: r = HANDLE_WM_LBUTTONUP(win, w, l, onLU); break;
        case WM_RBUTTONUP: r = HANDLE_WM_RBUTTONUP(win, w, l, onRU); break;
        case WM_MBUTTONUP: r = HANDLE_WM_MBUTTONUP(win, w, l, onMU); break;
#if defined HANDLE_WM_MOUSEWHEEL
        case WM_MOUSEWHEEL: r = HANDLE_WM_MOUSEWHEEL(win, w, l, onMW); break;
#endif
        case WM_INPUTLANGCHANGEREQUEST: return DefWindowProcW(win, msg, w, l); break;
        case WM_INPUTLANGCHANGE: return DefWindowProcW(win, msg, w, l); break;
        default: handled = 0;
        }
        if (handled)
                r = 0;
        else
                r = DefWindowProcW(win, msg, w, l);
        return r;
}

