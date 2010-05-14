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

#include <inttypes.h>
typedef intptr_t awcell;
typedef uintptr_t awucell;

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
	AW_KEY_A,
	AW_KEY_S,
	AW_KEY_D,
	AW_KEY_F,
	AW_KEY_H,
	AW_KEY_G,
	AW_KEY_Z,
	AW_KEY_X,
	AW_KEY_C,
	AW_KEY_V,
	AW_KEY_B,
	AW_KEY_Q,
	AW_KEY_W,
	AW_KEY_E,
	AW_KEY_R,
	AW_KEY_Y,
	AW_KEY_T,
	AW_KEY_1,
	AW_KEY_2,
	AW_KEY_3,
	AW_KEY_4,
	AW_KEY_6,
	AW_KEY_5,
	AW_KEY_EQUAL,
	AW_KEY_9,
	AW_KEY_7,
	AW_KEY_MINUS,
	AW_KEY_8,
	AW_KEY_0,
	AW_KEY_RIGHTBRACKET,
	AW_KEY_O,
	AW_KEY_U,
	AW_KEY_LEFTBRACKET,
	AW_KEY_I,
	AW_KEY_P,
	AW_KEY_L,
	AW_KEY_J,
	AW_KEY_QUOTE,
	AW_KEY_K,
	AW_KEY_SEMICOLON,
	AW_KEY_BACKSLASH,
	AW_KEY_COMMA,
	AW_KEY_SLASH,
	AW_KEY_N,
	AW_KEY_M,
	AW_KEY_PERIOD,
	AW_KEY_GRAVE,
	AW_KEY_KEYPADDECIMAL,
	AW_KEY_KEYPADMULTIPLY,
	AW_KEY_KEYPADPLUS,
	AW_KEY_KEYPADCLEAR,
	AW_KEY_KEYPADDIVIDE,
	AW_KEY_KEYPADENTER,
	AW_KEY_KEYPADMINUS,
	AW_KEY_KEYPADEQUALS,
	AW_KEY_KEYPAD0,
	AW_KEY_KEYPAD1,
	AW_KEY_KEYPAD2,
	AW_KEY_KEYPAD3,
	AW_KEY_KEYPAD4,
	AW_KEY_KEYPAD5,
	AW_KEY_KEYPAD6,
	AW_KEY_KEYPAD7,
	AW_KEY_KEYPAD8,
	AW_KEY_KEYPAD9,
	AW_KEY_RETURN,
	AW_KEY_TAB,
	AW_KEY_SPACE,
	AW_KEY_DELETE,
	AW_KEY_ESCAPE,
	AW_KEY_COMMAND,
	AW_KEY_SHIFT,
	AW_KEY_CAPSLOCK,
	AW_KEY_OPTION,
	AW_KEY_CONTROL,
	AW_KEY_RIGHTSHIFT,
	AW_KEY_RIGHTOPTION,
	AW_KEY_RIGHTCONTROL,
	AW_KEY_FUNCTION,
	AW_KEY_F17,
	AW_KEY_VOLUMEUP,
	AW_KEY_VOLUMEDOWN,
	AW_KEY_MUTE,
	AW_KEY_F18,
	AW_KEY_F19,
	AW_KEY_F20,
	AW_KEY_F5,
	AW_KEY_F6,
	AW_KEY_F7,
	AW_KEY_F3,
	AW_KEY_F8,
	AW_KEY_F9,
	AW_KEY_F11,
	AW_KEY_F13,
	AW_KEY_F16,
	AW_KEY_F14,
	AW_KEY_F10,
	AW_KEY_F12,
	AW_KEY_F15,
	AW_KEY_HELP,
	AW_KEY_HOME,
	AW_KEY_PAGEUP,
	AW_KEY_FORWARDDELETE,
	AW_KEY_F4,
	AW_KEY_END,
	AW_KEY_F2,
	AW_KEY_PAGEDOWN,
	AW_KEY_F1,
	AW_KEY_LEFTARROW,
	AW_KEY_RIGHTARROW,
	AW_KEY_DOWNARROW,
	AW_KEY_UPARROW,
	AW_KEY_MAX,
};

typedef struct _awEvent {
	awcell type;
	union {
		awcell p[2];
		struct _resize { awucell w, h; } resize;
		struct _down { awucell which; } down;
		struct _up { awucell which; } up;
		struct _motion { awcell x, y; } motion;
		struct _unicode { awucell which; } unicode;
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
EXPORTED const char * awKeyName(unsigned key);
EXPORTED const char * awResourcesPath();

EXPORTED ac * acNew(ac *);
EXPORTED void acDel(ac *);
EXPORTED void acSetInterval(ac *, int);
