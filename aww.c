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
#include "aw.h"
#include "awos.h"
#include "bit.h" 

#if !defined HANDLE_WM_MOUSEWHEEL // XXX mingw doesn't seem to define this one
#define HANDLE_WM_MOUSEWHEEL(hwnd,wParam,lParam,fn) ((fn)((hwnd),(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam),(int)(short)HIWORD(wParam),(UINT)(short)LOWORD(wParam)),0L)
#endif

static BOOL (APIENTRY *wglSwapInterval) (int interval) = 0;

static inline BOOL chk(BOOL b) {
#if !defined NDEBUG
        int err = GetLastError();
        if (err)
                report("failed %d", err);
        assert(!err);
#endif
        return b;
}

#define chk(x) (report("pre " #x), chk(1), report("checking " #x), chk(x))

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

int oswSetTitle(osw * w, const char * t) {
        WCHAR wt[1024];
        wide(wt, sizeof(wt), t);
        return chk(SetWindowTextW(w->win, wt));
}

static int openwin(osw * w, int x, int y, int width, int height, 
                    DWORD style, DWORD exstyle, osg * g) {
        RECT r;
        HWND win;
        w->style = style;
        r.left = x; r.top = y;
        r.right = x + width; r.bottom = y + height;
        chk(AdjustWindowRect(&r, style, FALSE));
        win = CreateWindowExW(exstyle, 
                              g->appname, g->appname, style,
                              r.left, r.top, 
                              r.right - r.left, r.bottom - r.top,
                              g->win,
                              NULL, GetModuleHandleW(NULL), w);
        if (win) {
                HDC dc = GetDC(win);
                setPF(dc);
                chk(ReleaseDC(win, dc));
                DragAcceptFiles(win, TRUE);
        }
        w->win = win;
        return win != 0;
}

static void dispatchmsgs(HWND win, unsigned type) {
        MSG msg;
        if (PeekMessageW(&msg, win, 0, 0, PM_REMOVE+type)) {
                TranslateMessage(&msg);
                chk(DispatchMessageW(&msg));
        }
}

#define EXITMSG (WM_USER+0x1024)

static DWORD __stdcall groupThread(LPVOID param) {
        osg * g = (osg *)param;;
        g->win = CreateWindowExW(0, g->appname, g->appname, 
                                 WS_POPUP, 0, 0, 0, 0, 0, 0, 
                                 GetModuleHandleW(NULL), 0);
//        ShowWindow(g->win, SW_SHOWNORMAL);
        chk(SetEvent(g->ready));
        while (1) {
                MSG msg;
                if (GetMessageW(&msg, 0, 0, 0)) {
                        TranslateMessage(&msg);
                        chk(DispatchMessageW(&msg));
                }
                if (msg.message == EXITMSG)
                        break;
        }
        chk(DestroyWindow(g->win));
        return 0; 
}

int osgInit(osg * g, const char * appname) {
        extern LRESULT CALLBACK handle(HWND win, UINT msg, WPARAM w, LPARAM l); 
        WNDCLASSW  wc;
        int ok;
        report("wide");
        wide(g->appname, sizeof(g->appname), appname);
        report("zme");
        ZeroMemory(&wc, sizeof(wc));
//        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.style += CS_OWNDC;
//        wc.style += CS_GLOBALCLASS;
        wc.lpfnWndProc = handle;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = g->appname;
        report("regclass");
        ok = 0 != RegisterClassW(&wc);
        report("cev");
        g->ready = CreateEvent(0,0,0,0);
        report("cth");
        g->thread = CreateThread(NULL, 4096, groupThread, g, 0, NULL);
        report("wso");
        chk(WaitForSingleObject(g->ready, INFINITE));
        report("ch");
        chk(CloseHandle(g->ready)); g->ready = 0;
        return ok;
}

int osgTerm(osg * g) {
        DWORD ret = 0;
        PostMessage(g->win, EXITMSG, 0, 0);
        ret = WaitForSingleObject(g->thread, INFINITE);
        assert(ret == WAIT_OBJECT_0);
        ret = 0 != chk(CloseHandle(g->thread));
        return ret;
}

void osgTick(osg * g) {
}

int oswInit(osw * w, osg * g, int x, int y, 
            int width, int height, int bl) {
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
        return openwin(w, x, y, width, height, style, exstyle, g);
}

int oswTerm(osw * w) {
        int ret;
        report("err0: %x", GetLastError());
        report("win: %x", w->win);
        ret = 0 != chk(DestroyWindow(w->win));
        report("err: %x", GetLastError());
        w->win = 0xdeadbeef;
        return ret;
}

int oswSwapBuffers(osw * w) {
        int ret;
        HDC dc = GetDC(w->win);
        ret = chk(SwapBuffers(dc));
        chk(ReleaseDC(w->win, dc));
        return ret;
}

int oswMakeCurrent(osw * w, osc * c) {
        int ret;
        HDC dc = GetDC(w->win);
        ret = chk(wglMakeCurrent(dc, c->ctx));
        if (!wglSwapInterval)
                wglSwapInterval = (void*)wglGetProcAddress("wglSwapIntervalEXT");
        chk(ReleaseDC(w->win, dc));
        return ret;
}

int oswClearCurrent(osw * w) {
        return chk(wglMakeCurrent(0, 0));
}

int oswShow(osw * w) {
        report("show");
        assert(!GetLastError());
        chk(ShowWindow(w->win, wmaximized(w)? SW_MAXIMIZE : SW_SHOWNORMAL));
        report("shown");
        return 1;
}

int oswHide(osw * w) {
        return chk(ShowWindow(w->win, SW_HIDE));
}

void oswPollEvent(osw * w) {
        dispatchmsgs(w->win, 0);
/*
  dispatch(w->win, PM_QS_POSTMESSAGE+PM_QS_SENDMESSAGE+PM_QS_PAINT);
  dispatch((HWND)-1, PM_QS_POSTMESSAGE+PM_QS_SENDMESSAGE+PM_QS_PAINT);
  dispatch((HWND)-1, 0);
  dispatch((HWND)0, PM_QS_POSTMESSAGE+PM_QS_SENDMESSAGE+PM_QS_PAINT);
*/
}

int oswSetSwapInterval(osw * w, int si) {
        return 1;
//        return wglSwapInterval? wglSwapInterval(si) : 1;
}

int oscInit(osc * c, osg * g, osc * share) {
        HGLRC ctx;
        HWND dummy = CreateWindowExW(0, g->appname, g->appname, 0, 0, 0, 0, 0, 0, 0, 
                                     GetModuleHandleW(NULL), 0);
        HDC dc = GetDC(dummy);
        setPF(dc);
        ctx = wglCreateContext(dc);
        chk(ReleaseDC(dummy, dc));
        chk(DestroyWindow(dummy));
        if (ctx && share) 
                chk(wglShareLists(share->ctx, ctx));
        c->ctx = ctx;
        return ctx != 0;
}

int oscTerm(osc * c) {
        int ret;
        ret = 0 != chk(wglDeleteContext(c->ctx));
        return ret;
}

int oswMaximize(osw * w) {
        // handled in oswShow()
        return 1;
}

int oswGeometry(osw * w, int x, int y, unsigned width, unsigned height) {
        RECT r;
        r.left = x; r.top = y;
        r.right = x + width; r.bottom = y + height;
        chk(AdjustWindowRect(&r, w->style, FALSE));
        return chk(MoveWindow(w->win, r.left, r.top, 
                              r.right - r.left, r.bottom - r.top, 
                              1));
}

void oswSetPointer(osw * w) {
        osp * p = wpointer(w);
        SetCursor(p? p->icon : 0);
}

void oswPointer(osw * w) {
        POINT p;
        chk(GetCursorPos(&p));
        chk(SetCursorPos(p.x, p.y));
}

int oswShowKeyboard(osw * w) {
        return 0;
}

int oswHideKeyboard(osw * w) {
        return 0;
}

int ospInit(osp * p, const void * vrgba, unsigned hotx, unsigned hoty) {
        HDC dc;
        HBITMAP bm;
        unsigned char * bits;
        int x, y;

        BITMAPV5HEADER bh;
        ICONINFO ii = {0};
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
        p->icon = CreateIconIndirect(&ii);
        chk(DeleteObject(ii.hbmMask));
        chk(DeleteObject(ii.hbmColor));
        return p->icon != 0;
}

int ospTerm(osp * p) {
        int ret;
        SetCursor(0);
        ret = chk(DestroyIcon(p->icon));
        return ret;
}

int main(int argc, char ** argv) {
        progrun(argc, argv);
        while (!progfinished())
                dispatch();
        return progterm();
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/

