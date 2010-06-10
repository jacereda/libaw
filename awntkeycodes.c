#include <windows.h>
#include "aw.h"

#if !defined MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR 2
#endif

unsigned mapkeycode(unsigned vk) {
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
