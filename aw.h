/*
Copyright (c) 2008, Jorge Acereda Maciá
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

enum {
  AW_EVENT_UNKNOWN,
  AW_EVENT_NONE,
  AW_EVENT_RESIZE,
  AW_EVENT_DOWN,
  AW_EVENT_UP,
  AW_EVENT_MOTION,
  AW_EVENT_CLOSE,
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
EXPORTED aw * awOpen(const char * title, int x, int y, int w, int h);
EXPORTED void awClose(aw *);
EXPORTED void awSwapBuffers(aw *);
EXPORTED void awPushCurrent(aw *);
EXPORTED void awPopCurrent(aw *);
EXPORTED const awEvent * awNextEvent(aw *);
