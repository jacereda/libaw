#include "aw.h"

awkey mapkey(unsigned k) {
        awkey ret;
        switch (k) {
	case 9: ret = AW_KEY_ESCAPE; break;
	case 10: ret = AW_KEY_1; break;
	case 11: ret = AW_KEY_2; break;
	case 12: ret = AW_KEY_3; break;
	case 13: ret = AW_KEY_4; break;
	case 14: ret = AW_KEY_5; break;
	case 15: ret = AW_KEY_6; break;
	case 16: ret = AW_KEY_7; break;
	case 17: ret = AW_KEY_8; break;
	case 18: ret = AW_KEY_9; break;
	case 19: ret = AW_KEY_0; break;
	case 20: ret = AW_KEY_MINUS; break;
	case 21: ret = AW_KEY_EQUAL; break;
	case 22: ret = AW_KEY_DELETE; break;
	case 23: ret = AW_KEY_TAB; break;
	case 24: ret = AW_KEY_Q; break;
	case 25: ret = AW_KEY_W; break;
	case 26: ret = AW_KEY_E; break;
	case 27: ret = AW_KEY_R; break;
	case 28: ret = AW_KEY_T; break;
	case 29: ret = AW_KEY_Y; break;
	case 30: ret = AW_KEY_U; break;
	case 31: ret = AW_KEY_I; break;
	case 32: ret = AW_KEY_O; break;
	case 33: ret = AW_KEY_P; break;
	case 34: ret = AW_KEY_LEFTBRACKET; break;
	case 35: ret = AW_KEY_RIGHTBRACKET; break;
	case 36: ret = AW_KEY_RETURN; break;
	case 37: ret = AW_KEY_CONTROL; break;
	case 38: ret = AW_KEY_A; break;
	case 39: ret = AW_KEY_S; break;
	case 40: ret = AW_KEY_D; break;
	case 41: ret = AW_KEY_F; break;
	case 42: ret = AW_KEY_G; break;
	case 43: ret = AW_KEY_H; break;
	case 44: ret = AW_KEY_J; break;
	case 45: ret = AW_KEY_K; break;
	case 46: ret = AW_KEY_L; break;
	case 47: ret = AW_KEY_SEMICOLON; break;
	case 48: ret = AW_KEY_QUOTE; break;
	case 49: ret = AW_KEY_GRAVE; break;
	case 50: ret = AW_KEY_SHIFT; break;
	case 51: ret = AW_KEY_BACKSLASH; break;
	case 52: ret = AW_KEY_Z; break;
	case 53: ret = AW_KEY_X; break;
	case 54: ret = AW_KEY_C; break;
	case 55: ret = AW_KEY_V; break;
	case 56: ret = AW_KEY_B; break;
	case 57: ret = AW_KEY_N; break;
	case 58: ret = AW_KEY_M; break;
	case 59: ret = AW_KEY_COMMA; break;
	case 60: ret = AW_KEY_PERIOD; break;
	case 61: ret = AW_KEY_SLASH; break;
	case 62: ret = AW_KEY_RIGHTSHIFT; break;
	case 63: ret = AW_KEY_KEYPADMULTIPLY; break;
	case 64: ret = AW_KEY_OPTION; break;
	case 65: ret = AW_KEY_SPACE; break;

	case 67: ret = AW_KEY_F1; break;
	case 68: ret = AW_KEY_F2; break;
	case 69: ret = AW_KEY_F3; break;
	case 70: ret = AW_KEY_F4; break;
	case 71: ret = AW_KEY_F5; break;
	case 72: ret = AW_KEY_F6; break;
	case 73: ret = AW_KEY_F7; break;
	case 74: ret = AW_KEY_F8; break;
	case 75: ret = AW_KEY_F9; break;
	case 76: ret = AW_KEY_F10; break;

	case 79: ret = AW_KEY_KEYPAD7; break;
	case 80: ret = AW_KEY_KEYPAD8; break;
	case 81: ret = AW_KEY_KEYPAD9; break;
	case 82: ret = AW_KEY_KEYPADMINUS; break;
	case 83: ret = AW_KEY_KEYPAD4; break;
	case 84: ret = AW_KEY_KEYPAD5; break;
	case 85: ret = AW_KEY_KEYPAD6; break;
	case 86: ret = AW_KEY_KEYPADPLUS; break;
	case 87: ret = AW_KEY_KEYPAD1; break;
	case 88: ret = AW_KEY_KEYPAD2; break;
	case 89: ret = AW_KEY_KEYPAD3; break;
	case 90: ret = AW_KEY_KEYPAD0; break;
	case 91: ret = AW_KEY_KEYPADDECIMAL; break;

	case 104: ret = AW_KEY_KEYPADENTER; break;

	case 106: ret = AW_KEY_KEYPADDIVIDE; break;

	case 110: ret = AW_KEY_HOME; break;

	case 113: ret = AW_KEY_LEFTARROW; break;
	case 114: ret = AW_KEY_RIGHTARROW; break;
	case 115: ret = AW_KEY_END; break;
	case 116: ret = AW_KEY_DOWNARROW; break;
	case 111: ret = AW_KEY_UPARROW; break;
	case 112: ret = AW_KEY_PAGEUP; break;

	case 117: ret = AW_KEY_PAGEDOWN; break;

	case 119: ret = AW_KEY_FORWARDDELETE; break;

	case 133: ret = AW_KEY_COMMAND; break;
/*
  case XK_Clear: ret = AW_KEY_KEYPADCLEAR; break;
  case XK_KP_Equal: ret = AW_KEY_KEYPADEQUALS; break;
  case XK_Caps_Lock: ret = AW_KEY_CAPSLOCK; break;
  case XK_Alt_R: ret = AW_KEY_RIGHTOPTION; break;
  case XK_Control_R: ret = AW_KEY_RIGHTCONTROL; break;
//	case XK_function: ret = AW_KEY_FUNCTION; break;
case XK_F17: ret = AW_KEY_F17; break;
//	case XK_VolumeUp: ret = AW_KEY_VOLUMEUP; break;
//	case XK_VolumeDown: ret = AW_KEY_VOLUMEDOWN; break;
//	case XK_Mute: ret = AW_KEY_MUTE; break;
case XK_F18: ret = AW_KEY_F18; break;
case XK_F19: ret = AW_KEY_F19; break;
case XK_F20: ret = AW_KEY_F20; break;

case XK_F11: ret = AW_KEY_F11; break;
case XK_F13: ret = AW_KEY_F13; break;
case XK_F16: ret = AW_KEY_F16; break;
case XK_F14: ret = AW_KEY_F14; break;
case XK_F12: ret = AW_KEY_F12; break;
case XK_F15: ret = AW_KEY_F15; break;
case XK_Help: ret = AW_KEY_HELP; break;
*/
        default: ret = AW_KEY_NONE;
        }
        return ret;
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/

