/*
  Copyright (c) 2008-2009, Jorge Acereda Maciá
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:
        
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
        
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the author nor the names of its contributors
  may be used to endorse or promote products derived from this
  software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define _WIN32_WINNT 0x0500
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include "aw.h"
#include "awos.h"

#if !defined HANDLE_WM_MOUSEWHEEL // XXX mingw doesn't seem to define this one
#define HANDLE_WM_MOUSEWHEEL(hwnd,wParam,lParam,fn) ((fn)((hwnd),(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam),(int)(short)HIWORD(wParam),(UINT)(short)LOWORD(wParam)),0L)
#endif


#if !defined MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR 2
#endif

struct _aw {
        awHeader hdr;
        HWND win;
        int lastmx, lastmy;
};

struct _ac {
        acHeader hdr;
        HGLRC ctx;
};

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
        if (x != w->lastmx || y != w->lastmy)
                got(w, AW_EVENT_MOTION, x, y);
        w->lastmx = x; w->lastmy = y;
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

static unsigned vk2aw(UINT vk) {
        int ret;
        switch (vk) {
	case 'A': ret = AW_KEY_A; break;
	case 'S': ret = AW_KEY_S; break;
	case 'D': ret = AW_KEY_D; break;
	case 'F': ret = AW_KEY_F; break;
	case 'H': ret = AW_KEY_H; break;
	case 'G': ret = AW_KEY_G; break;
	case 'Z': ret = AW_KEY_Z; break;
	case 'X': ret = AW_KEY_X; break;
	case 'C': ret = AW_KEY_C; break;
	case 'V': ret = AW_KEY_V; break;
	case 'B': ret = AW_KEY_B; break;
	case 'Q': ret = AW_KEY_Q; break;
	case 'W': ret = AW_KEY_W; break;
	case 'E': ret = AW_KEY_E; break;
	case 'R': ret = AW_KEY_R; break;
	case 'Y': ret = AW_KEY_Y; break;
	case 'T': ret = AW_KEY_T; break;
	case '1': ret = AW_KEY_1; break;
	case '2': ret = AW_KEY_2; break;
	case '3': ret = AW_KEY_3; break;
	case '4': ret = AW_KEY_4; break;
	case '6': ret = AW_KEY_6; break;
	case '5': ret = AW_KEY_5; break;
	case VK_OEM_PLUS: ret = AW_KEY_EQUAL; break;
	case '9': ret = AW_KEY_9; break;
	case '7': ret = AW_KEY_7; break;
	case VK_OEM_MINUS: ret = AW_KEY_MINUS; break;
	case '8': ret = AW_KEY_8; break;
	case '0': ret = AW_KEY_0; break;
	case VK_OEM_6: ret = AW_KEY_RIGHTBRACKET; break;
	case 'O': ret = AW_KEY_O; break;
	case 'U': ret = AW_KEY_U; break;
	case VK_OEM_4: ret = AW_KEY_LEFTBRACKET; break;
	case 'I': ret = AW_KEY_I; break;
	case 'P': ret = AW_KEY_P; break;
	case 'L': ret = AW_KEY_L; break;
	case 'J': ret = AW_KEY_J; break;
	case VK_OEM_7: ret = AW_KEY_QUOTE; break;
	case 'K': ret = AW_KEY_K; break;
	case VK_OEM_1: ret = AW_KEY_SEMICOLON; break;
	case VK_OEM_5: ret = AW_KEY_BACKSLASH; break;
	case VK_OEM_COMMA: ret = AW_KEY_COMMA; break;
	case VK_OEM_2: ret = AW_KEY_SLASH; break;
	case 'N': ret = AW_KEY_N; break;
	case 'M': ret = AW_KEY_M; break;
	case VK_OEM_PERIOD: ret = AW_KEY_PERIOD; break;
	case VK_OEM_3: ret = AW_KEY_GRAVE; break;
	case VK_DECIMAL: ret = AW_KEY_KEYPADDECIMAL; break;
	case VK_MULTIPLY: ret = AW_KEY_KEYPADMULTIPLY; break;
	case VK_ADD: ret = AW_KEY_KEYPADPLUS; break;
//	case 'K'eypadClear: ret = AW_KEY_KEYPADCLEAR; break;
	case VK_DIVIDE: ret = AW_KEY_KEYPADDIVIDE; break;
//	case 'K'eypadEnter: ret = AW_KEY_KEYPADENTER; break;
	case VK_SUBTRACT: ret = AW_KEY_KEYPADMINUS; break;
//	case 'K'eypadEquals: ret = AW_KEY_KEYPADEQUALS; break;
	case VK_NUMPAD0: ret = AW_KEY_KEYPAD0; break;
        case VK_NUMPAD1: ret = AW_KEY_KEYPAD1; break;
	case VK_NUMPAD2: ret = AW_KEY_KEYPAD2; break;
	case VK_NUMPAD3: ret = AW_KEY_KEYPAD3; break;
	case VK_NUMPAD4: ret = AW_KEY_KEYPAD4; break;
	case VK_NUMPAD5: ret = AW_KEY_KEYPAD5; break;
	case VK_NUMPAD6: ret = AW_KEY_KEYPAD6; break;
	case VK_NUMPAD7: ret = AW_KEY_KEYPAD7; break;
	case VK_NUMPAD8: ret = AW_KEY_KEYPAD8; break;
	case VK_NUMPAD9: ret = AW_KEY_KEYPAD9; break;
	case VK_RETURN: ret = AW_KEY_RETURN; break;
	case VK_TAB: ret = AW_KEY_TAB; break;
	case VK_SPACE: ret = AW_KEY_SPACE; break;
	case VK_BACK: ret = AW_KEY_DELETE; break;
	case VK_ESCAPE: ret = AW_KEY_ESCAPE; break;
	case VK_MENU: ret = AW_KEY_OPTION; break;
        case VK_LSHIFT:
	case VK_SHIFT: ret = AW_KEY_SHIFT; break;
        case VK_LWIN:
        case VK_RWIN: ret = AW_KEY_COMMAND; break;
	case VK_CAPITAL: ret = AW_KEY_CAPSLOCK; break;
	case VK_CONTROL: ret = AW_KEY_CONTROL; break;
	case VK_RSHIFT: ret = AW_KEY_RIGHTSHIFT; break;
	case VK_RMENU: ret = AW_KEY_RIGHTOPTION; break;
	case VK_RCONTROL: ret = AW_KEY_RIGHTCONTROL; break;
	case VK_F17: ret = AW_KEY_F17; break;
	case VK_VOLUME_UP: ret = AW_KEY_VOLUMEUP; break;
	case VK_VOLUME_DOWN: ret = AW_KEY_VOLUMEDOWN; break;
	case VK_VOLUME_MUTE: ret = AW_KEY_MUTE; break;
	case VK_F18: ret = AW_KEY_F18; break;
	case VK_F19: ret = AW_KEY_F19; break;
	case VK_F20: ret = AW_KEY_F20; break;
	case VK_F5: ret = AW_KEY_F5; break;
	case VK_F6: ret = AW_KEY_F6; break;
	case VK_F7: ret = AW_KEY_F7; break;
	case VK_F3: ret = AW_KEY_F3; break;
	case VK_F8: ret = AW_KEY_F8; break;
	case VK_F9: ret = AW_KEY_F9; break;
	case VK_F11: ret = AW_KEY_F11; break;
	case VK_F13: ret = AW_KEY_F13; break;
	case VK_F16: ret = AW_KEY_F16; break;
	case VK_F14: ret = AW_KEY_F14; break;
	case VK_F10: ret = AW_KEY_F10; break;
	case VK_F12: ret = AW_KEY_F12; break;
	case VK_F15: ret = AW_KEY_F15; break;
	case VK_HELP: ret = AW_KEY_HELP; break;
	case VK_HOME: ret = AW_KEY_HOME; break;
	case VK_PRIOR: ret = AW_KEY_PAGEUP; break;
        case VK_INSERT: ret = AW_KEY_FUNCTION; break;
	case VK_DELETE: ret = AW_KEY_FORWARDDELETE; break;
	case VK_F4: ret = AW_KEY_F4; break;
	case VK_END: ret = AW_KEY_END; break;
	case VK_F2: ret = AW_KEY_F2; break;
	case VK_NEXT: ret = AW_KEY_PAGEDOWN; break;
	case VK_F1: ret = AW_KEY_F1; break;
	case VK_LEFT: ret = AW_KEY_LEFTARROW; break;
	case VK_RIGHT: ret = AW_KEY_RIGHTARROW; break;
	case VK_DOWN: ret = AW_KEY_DOWNARROW; break;
	case VK_UP: ret = AW_KEY_UPARROW; break;
        default:
                ret = towlower(MapVirtualKeyW(vk, MAPVK_VK_TO_CHAR));
                break;
        }
        return ret;
}

static void onSysKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        if (vk == VK_MENU)
                wgot(win, AW_EVENT_DOWN, vk2aw(vk), 0);
}

static void onSysKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        if (vk == VK_MENU)
                wgot(win, AW_EVENT_UP, vk2aw(vk), 0);
}

static void onKeyDown(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        wgot(win, AW_EVENT_DOWN, vk2aw(vk), 0);
}

static void onKeyUp(HWND win, UINT vk, BOOL down, int repeats, UINT flags) {
        wgot(win, AW_EVENT_UP, vk2aw(vk), 0);
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
        case WM_MOUSEWHEEL: r = HANDLE_WM_MOUSEWHEEL(win, w, l, onMW); break;
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

static BOOL (APIENTRY *wglSwapInterval) (int interval) = 0;

int awosInit() {
        WNDCLASSW  wc;
        memset(&wc, 0, sizeof(WNDCLASS));
        wc.style            = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc      = handle;
        wc.hInstance        = GetModuleHandleW(NULL);
        wc.lpszClassName    = L"AW";
        return 0 != RegisterClassW(&wc);
}

int awosEnd() {
        return 1;
}

static void setPF(HDC dc) {
        PIXELFORMATDESCRIPTOR pfd = { 
                sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
                1,                     // version number 
                PFD_DRAW_TO_WINDOW |   // support window 
                PFD_SUPPORT_OPENGL |   // support OpenGL 
                PFD_DOUBLEBUFFER,      // double buffered 
                PFD_TYPE_RGBA,         // RGBA type 
                24,                    // 24-bit color depth 
                0, 0, 0, 0, 0, 0,      // color bits ignored 
                0,                     // no alpha buffer 
                0,                     // shift bit ignored 
                0,                     // no accumulation buffer 
                0, 0, 0, 0,            // accum bits ignored 
                32,                    // 32-bit z-buffer 
                0,                     // no stencil buffer 
                0,                     // no auxiliary buffer 
                PFD_MAIN_PLANE,        // main layer 
                0,                     // reserved 
                0, 0, 0                // layer masks ignored 
        }; 
        SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd); 
}

int awosSetTitle(aw * w, const char * t) {
        WCHAR wt[1024];
        MultiByteToWideChar(CP_UTF8, 0, t, strlen(t)+1, wt, sizeof(wt));
        return SetWindowTextW(w->win, wt);
        return 1;
}

static aw * openwin(int x, int y, int width, int height, 
                    DWORD style, DWORD exstyle) {
        aw * w = calloc(1, sizeof(*w));
        RECT r;
        HWND win;
        r.left = x; r.top = y;
        r.right = x + width; r.bottom = y + height;
        AdjustWindowRect(&r, style, FALSE);
        win = CreateWindowExW(exstyle, 
                              L"AW", L"AW", style,
                              r.left, r.top, 
                              r.right - r.left, r.bottom - r.top,
                              NULL, NULL, GetModuleHandle(NULL), w);
        if (win) {
                HDC dc = GetDC(win);
                setPF(dc);
                ReleaseDC(win, dc);
        }
        if (w)
                w->win = win;
        else
                free(w);
        return w;
}

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
        aw * w;
        DWORD style, exstyle;
#if 0
        if (fs) {
                RECT rect;
                GetWindowRect(GetDesktopWindow(), &rect);
                width = rect.right;
                height = rect.bottom;
        }
#endif
        if (bl) {
                style = WS_POPUP;
                exstyle = WS_EX_TOPMOST; 
        }
        else {
                style = 0
                        | WS_OVERLAPPED
                        | WS_CAPTION 
                        | WS_SYSMENU 
                        | WS_SIZEBOX
                        | WS_MINIMIZEBOX
                        | WS_MAXIMIZEBOX
                        ;
                exstyle = 0;
        }
//  if (fs)
//      style += WS_MAXIMIZE;
        w = openwin(x, y, width, height, style, exstyle);
        return w;
}

int awosClose(aw * w) {
        int ret = 1;
        if (w->win) ret &= 0 != DestroyWindow(w->win);
        free(w);
        return ret;
}

int awosSwapBuffers(aw * w) {
        int ret;
        HDC dc = GetDC(w->win);
        ret = SwapBuffers(dc);
        ReleaseDC(w->win, dc);
        return ret;
}

int awosMakeCurrent(aw * w, ac * c) {
        int ret;
        HDC dc = GetDC(w->win);
        ret = wglMakeCurrent(dc, c->ctx);
        if (!wglSwapInterval)
                wglSwapInterval = (void*)wglGetProcAddress("wglSwapIntervalEXT");
        ReleaseDC(w->win, dc);
        return ret;
}

int awosClearCurrent(aw * w) {
        return wglMakeCurrent(0, 0);
}

int awosShow(aw * w) {
        ShowWindow(w->win, w->hdr.fullscreen? SW_MAXIMIZE : SW_SHOWNORMAL);
        return 1;
}

int awosHide(aw * w) {
        ShowWindow(w->win, SW_HIDE);
        return 1;
}

static void dispatch(HWND win) {
        MSG msg;
        if (PeekMessageW(&msg, win, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
        }
}

void awosPollEvent(aw * w) {
        dispatch(w->win);
}

int awosSetSwapInterval(aw * w, int si) {
        return wglSwapInterval? wglSwapInterval(si) : 1;
//  return 1;
}

ac * acosNew(ac * share) {
        ac * c = 0;
        HGLRC ctx;
        aw * dummy = awosOpen(0, 0, 0, 0, 0, 0);
        HDC dc = GetDC(dummy->win);
        setPF(dc);
        ctx = wglCreateContext(dc);
        ReleaseDC(dummy->win, dc);
        awosClose(dummy);
        if (ctx && share) 
                wglShareLists(share->ctx, ctx);
        if (ctx)
                c = calloc(1, sizeof(*c));
        if (c)
                c->ctx = ctx;
        return c;
}

int acosDel(ac * c) {
        int ret = 0 != wglDeleteContext(c->ctx);
        free(c);
        return ret;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
