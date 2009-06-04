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

enum {
	AW_EVENT_UNKNOWN,
	AW_EVENT_NONE,
	AW_EVENT_RESIZE,
	AW_EVENT_DOWN,
	AW_EVENT_UP,
	AW_EVENT_UNICODE,
	AW_EVENT_MOTION,
	AW_EVENT_CLOSE,
};

enum {
	AW_KEY_NONE = 0x40000000,
	AW_KEY_MOUSEWHEELUP, 
	AW_KEY_MOUSEWHEELDOWN,
	AW_KEY_MOUSELEFT,
	AW_KEY_MOUSERIGHT,
	AW_KEY_MOUSEMIDDLE,
	AW_KEY_SHIFT,
	AW_KEY_ALT,
	AW_KEY_CONTROL,
	AW_KEY_META,
	AW_KEY_CURSORUP,
	AW_KEY_CURSORDOWN,
	AW_KEY_CURSORLEFT,
	AW_KEY_CURSORRIGHT,
	AW_KEY_PAGEUP,
	AW_KEY_PAGEDOWN,
	AW_KEY_HOME,
	AW_KEY_END,
	AW_KEY_MAX,
	AW_KEY_BACKSPACE = 8,
	AW_KEY_RETURN = 10,
};

typedef struct _awEvent {
	int type;
	union {
		int p[2];
		struct _resize { unsigned w, h; } resize;
		struct _down { unsigned which; } down;
		struct _up { unsigned which; } up;
		struct _motion { int x, y; } motion;
		struct _unicode { unsigned which; } unicode;
	} u;
} awEvent;

#if defined(__GNUC__)
#define EXPORTED extern __attribute__((visibility("default")))
#else
#define EXPORTED extern __declspec(dllexport)
#endif

typedef struct _aw aw;
typedef struct _ac ac;

EXPORTED int awInit(void);
EXPORTED void awEnd(void);
EXPORTED aw * awOpen(int x, int y, unsigned w, unsigned h);
EXPORTED aw * awOpenBorderless(int x, int y, unsigned w, unsigned h);
EXPORTED aw * awOpenFullscreen(void);
EXPORTED aw * awOpenMaximized(void);
EXPORTED void awSetTitle(aw *, const char *);
EXPORTED void awClose(aw *);
EXPORTED void awSwapBuffers(aw *);
EXPORTED void awMakeCurrent(aw *, ac *);
EXPORTED const awEvent * awNextEvent(aw *);
EXPORTED unsigned awWidth(aw *);
EXPORTED unsigned awHeight(aw *);
EXPORTED int awMouseX(aw *);
EXPORTED int awMouseY(aw *);
EXPORTED int awPressed(aw *, unsigned key);

EXPORTED ac * acNew(ac *);
EXPORTED void acDel(ac *);
EXPORTED void acSetInterval(ac *, int);
