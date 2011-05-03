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

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include "aw.h"
#include "awos.h"
#include "bit.h" 

#if !defined HANDLE_WM_MOUSEWHEEL // XXX mingw doesn't seem to define this one
#define HANDLE_WM_MOUSEWHEEL(hwnd,wParam,lParam,fn) ((fn)((hwnd),(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam),(int)(short)HIWORD(wParam),(UINT)(short)LOWORD(wParam)),0L)
#endif

struct _ag {
        agHeader hdr;
        HANDLE ready;
        HANDLE thread;
        HWND win;
        WCHAR appname[256];
};

struct _aw {
        awHeader hdr;
        HWND win;
        DWORD style;
};

struct _ac {
        acHeader hdr;
        HGLRC ctx;
};

struct _ap {
        apHeader hdr;
        HCURSOR icon;
};

static BOOL (APIENTRY *wglSwapInterval) (int interval) = 0;

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

static void wide(WCHAR * d, size_t dsz, const char * t) {
        MultiByteToWideChar(CP_UTF8, 0, t, -1, d, (int)dsz);
}

int awosSetTitle(aw * w, const char * t) {
        WCHAR wt[1024];
        wide(wt, sizeof(wt), t);
        return SetWindowTextW(w->win, wt);
}

static aw * openwin(int x, int y, int width, int height, 
                    DWORD style, DWORD exstyle, ag * g) {
        aw * w = calloc(1, sizeof(*w));
        RECT r;
        HWND win;
        w->style = style;
        r.left = x; r.top = y;
        r.right = x + width; r.bottom = y + height;
        AdjustWindowRect(&r, style, FALSE);
        win = CreateWindowExW(exstyle, 
                              g->appname, g->appname, style,
                              r.left, r.top, 
                              r.right - r.left, r.bottom - r.top,
                              g->win,
                              NULL, GetModuleHandleW(NULL), w);
        if (win) {
                HDC dc = GetDC(win);
                setPF(dc);
                ReleaseDC(win, dc);
                DragAcceptFiles(win, TRUE);
        }
        if (w)
                w->win = win;
        else
                free(w);
        return w;
}

static void dispatch(HWND win, unsigned type) {
        MSG msg;
        if (PeekMessageW(&msg, win, 0, 0, PM_REMOVE+type)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
        }
}

#define EXITMSG (WM_USER+0x1024)

static DWORD __stdcall groupThread(LPVOID param) {
        ag * g = (ag *)param;;
        g->win = CreateWindowExW(0, g->appname, g->appname, 
                                 WS_POPUP, 0, 0, 0, 0, 0, 0, 
                                 GetModuleHandleW(NULL), 0);
//        ShowWindow(g->win, SW_SHOWNORMAL);
        SetEvent(g->ready);
        while (1) {
                MSG msg;
                if (GetMessageW(&msg, 0, 0, 0)) {
                        TranslateMessage(&msg);
                        DispatchMessageW(&msg);
                }
                if (msg.message == EXITMSG)
                        break;
        }
        DestroyWindow(g->win);
        return 0; 
}


ag * agosNew(const char * appname) {
        extern LRESULT CALLBACK handle(HWND win, UINT msg, WPARAM w, LPARAM l); 
        WNDCLASSW  wc;
        int ok;
        ag * g = calloc(1, sizeof(*g));
        wide(g->appname, sizeof(g->appname), appname);
        ZeroMemory(&wc, sizeof(wc));
//        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.style += CS_OWNDC;
//        wc.style += CS_GLOBALCLASS;
        wc.lpfnWndProc = handle;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = g->appname;
        ok = 0 != RegisterClassW(&wc);
        g->ready = CreateEvent(0,0,0,0);
        g->thread = CreateThread(NULL, 4096, groupThread, g, 0, NULL);
        WaitForSingleObject(g->ready, INFINITE);
        CloseHandle(g->ready); g->ready = 0;
        return g;
}

int agosDel(ag * g) {
        DWORD ret = 0;
        PostMessage(g->win, EXITMSG, 0, 0);
        ret = WaitForSingleObject(g->thread, INFINITE);
        assert(ret == WAIT_OBJECT_0);
        ret = 0 != CloseHandle(g->thread);
        return ret;
}

aw * awosOpen(ag * g, int x, int y, int width, int height, int fs, int bl) {
        aw * w;
        DWORD style, exstyle;
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
                        | 0;
                exstyle = WS_EX_APPWINDOW;
        }
        w = openwin(x, y, width, height, style, exstyle, g);
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

void awosPollEvent(aw * w) {
        dispatch(w->win, 0);
/*
  dispatch(w->win, PM_QS_POSTMESSAGE+PM_QS_SENDMESSAGE+PM_QS_PAINT);
  dispatch((HWND)-1, PM_QS_POSTMESSAGE+PM_QS_SENDMESSAGE+PM_QS_PAINT);
  dispatch((HWND)-1, 0);
  dispatch((HWND)0, PM_QS_POSTMESSAGE+PM_QS_SENDMESSAGE+PM_QS_PAINT);
*/
}

void awosThreadEvents() {
        dispatch((HWND)0, 0);
//        report("qs %x", GetQueueStatus(QS_ALLEVENTS));
}

int awosSetSwapInterval(aw * w, int si) {
        return wglSwapInterval? wglSwapInterval(si) : 1;
}

ac * acosNew(ag * g, ac * share) {
        ac * c = 0;
        HGLRC ctx;
        HWND dummy = CreateWindowExW(0, g->appname, g->appname, 0, 0, 0, 0, 0, 0, 0, 
                                     GetModuleHandleW(NULL), 0);
        HDC dc = GetDC(dummy);
        setPF(dc);
        ctx = wglCreateContext(dc);
        ReleaseDC(dummy, dc);
        DestroyWindow(dummy);
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

int awosGeometry(aw * w, int x, int y, unsigned width, unsigned height) {
        RECT r;
        r.left = x; r.top = y;
        r.right = x + width; r.bottom = y + height;
        AdjustWindowRect(&r, w->style, FALSE);
        return MoveWindow(w->win, r.left, r.top, 
                   r.right - r.left, r.bottom - r.top, 
                   1);
}

void awosSetPointer(aw * w) {
        if (w->hdr.pointer)
                SetCursor(w->hdr.pointer->icon);
        else
                SetCursor(0);
}

void awosPointer(aw * w) {
        POINT p;
        GetCursorPos(&p);
        SetCursorPos(p.x, p.y);
}

ap * aposNew(const void * vrgba, unsigned hotx, unsigned hoty) {
        HDC dc;
        HBITMAP bm;
        unsigned char * bits;
        int x, y;

        BITMAPV5HEADER bh;
        ICONINFO ii = {0};
        HCURSOR cur = {0};
        ap * ret = calloc(1, sizeof(*ret));
        int w = 32;
        int h = 32;
        
        ZeroMemory(&bh, sizeof(BITMAPV5HEADER));
        bh.bV5Size = sizeof(bh);
        bh.bV5Width = w;
        bh.bV5Height = -h;
        bh.bV5Planes = 1;
        bh.bV5BitCount = 32;
        bh.bV5Compression = BI_RGB;
        bh.bV5AlphaMask = 0xff000000;
        bh.bV5BlueMask = 0x00ff0000;
        bh.bV5GreenMask = 0x0000ff00;
        bh.bV5RedMask = 0x000000ff;
        
        dc = GetDC(0);
        bm = CreateDIBSection(dc, (BITMAPINFO*)&bh, DIB_RGB_COLORS, (void **)&bits, 0, 0);
        ReleaseDC(0, dc);
        
        memcpy(bits, vrgba, w * h * 4);

        // Change red <-> blue
        for (y = 0; y < h; y++)
                for (x = 0; x < w; x++) {
                        unsigned c = (y*w + x) * 4;
                        unsigned aux = bits[c + 0];
                        bits[c + 0] = bits[c + 2];
                        bits[c + 2] = aux;
                }

        ii.fIcon = FALSE;
        ii.xHotspot = hotx;
        ii.yHotspot = hoty;
        ii.hbmColor = bm;
        ii.hbmMask = CreateBitmap(w, h, 1, 1, NULL);
        ret->icon = CreateIconIndirect(&ii);
        DeleteObject(ii.hbmMask);
        DeleteObject(ii.hbmColor);
        return ret;
}

int aposDel(ap * p) {
        int ret;
        SetCursor(0);
        ret = DestroyIcon(p->icon);
        free(p);
        return ret;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/

