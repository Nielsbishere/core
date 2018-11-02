#pragma once
#include "template/enum.h"

#include <Windows.h>
#undef min
#undef max
#undef ERROR
#undef near
#undef far

namespace oi {

	namespace wc {

		class Window;

		struct WWindow {

			HINSTANCE instance;
			HWND window;

			static Window *getByHandle(HWND hwnd);
			static LRESULT CALLBACK windowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

		};

		DEnum(WKey, WPARAM, Undefined = 0, Zero = 48, One = 49, Two = 50, Three = 51, Four = 52, Five = 53, Six = 54, Seven = 55, Eight = 56, Nine = 57, A = 65, B = 66, C = 67, D = 68, E = 69,
			F = 70, G = 71, H = 72, I = 73, J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84, U = 85, V = 86, W = 87, X = 88,
			Y = 89, Z = 90, Minus = VK_OEM_MINUS, Equals = VK_OEM_NEC_EQUAL, Left_brace = VK_OEM_4, Right_brace = VK_OEM_6, Semicolon = VK_OEM_1, Apostrophe = VK_OEM_7,
			Tilde = VK_OEM_3, Backslash = VK_OEM_5, Comma = VK_OEM_COMMA, Period = VK_OEM_PERIOD, Slash = VK_OEM_2, Space = VK_SPACE, n0 = VK_NUMPAD0, n1 = VK_NUMPAD1,
			n2 = VK_NUMPAD2, n3 = VK_NUMPAD3, n4 = VK_NUMPAD4, n5 = VK_NUMPAD5, n6 = VK_NUMPAD6, n7 = VK_NUMPAD7, n8 = VK_NUMPAD8, n9 = VK_NUMPAD9, Subtract = VK_SUBTRACT,
			Add = VK_ADD, Decimal = VK_DECIMAL, Multiply = VK_MULTIPLY, Divide = VK_DIVIDE, F1 = VK_F1, F2 = VK_F2, F3 = VK_F3, F4 = VK_F4, F5 = VK_F5, F6 = VK_F6, F7 = VK_F7,
			F8 = VK_F8, F9 = VK_F9, F10 = VK_F10, F11 = VK_F11, F12 = VK_F12, F13 = VK_F13, F14 = VK_F14, F15 = VK_F15, F16 = VK_F16, F17 = VK_F17, F18 = VK_F18, F19 = VK_F19,
			F20 = VK_F20, F21 = VK_F21, F22 = VK_F22, F23 = VK_F23, F24 = VK_F24, Up = VK_UP, Down = VK_DOWN, Left = VK_LEFT, Right = VK_RIGHT, Page_up = VK_NEXT, Page_down = VK_PRIOR,
			Home = VK_HOME, End = VK_END, Insert = VK_INSERT, Delete = VK_DELETE, Scroll_lock = VK_SCROLL, Num_lock = VK_NUMLOCK, Caps_lock = VK_CAPITAL,
			Tab = VK_TAB, Enter = VK_RETURN, Backspace = VK_BACK, Esc = VK_ESCAPE,
			Plus = VK_OEM_PLUS, Left_shift = VK_SHIFT, Right_shift = VK_SHIFT, Left_ctrl = VK_CONTROL, Right_ctrl = VK_CONTROL
		);

	}

}