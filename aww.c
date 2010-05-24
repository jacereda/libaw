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

struct _aw {
        awHeader hdr;
        HWND win;
        int lastmx, lastmy;
};

struct _ac {
        acHeader hdr;
        HGLRC ctx;
};

static BOOL (APIENTRY *wglSwapInterval) (int interval) = 0;

int awosInit() {
        extern LONG WINAPI handle(HWND win, UINT msg, WPARAM w, LPARAM l); 
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
