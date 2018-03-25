#pragma once

#include <template/enum.h>

namespace oi {

	namespace wc {

		DEnum(ControllerButton, u32, Undefined = 0, Cross = 1, Square = 2, Triangle = 3, Circle = 4, Down = 5, Left = 6, Up = 7, Right = 8, L1 = 9, R1 = 10, L3 = 11, R3 = 12, Share = 13, Options = 14);
		DEnum(ControllerAxis, u32, Undefined = 0, Lx = 1, Ly = 2, Rx = 3, Ry = 4, L2 = 5, R2 = 6);

		DEnum(Key, u32, Undefined = 0, Zero = 1, One = 2, Two = 3, Three = 4, Four = 5, Five = 6, Six = 7, Seven = 8, Eight = 9, Nine = 10, A = 11, B = 12, C = 13, D = 14, E = 15,
			F = 16, G = 17, H = 18, I = 19, J = 20, K = 21, L = 22, M = 23, N = 24, O = 25, P = 26, Q = 27, R = 28, S = 29, T = 30, U = 31, V = 32, W = 33, X = 34,
			Y = 35, Z = 36, Minus = 37, Equals = 38, Left_brace = 39, Right_brace = 40, Semicolon = 41, Apostrophe = 42,
			Tilde = 43, Backslash = 44, Comma = 45, Period = 46, Slash = 47, Space = 48, n0 = 49, n1 = 50,
			n2 = 51, n3 = 52, n4 = 53, n5 = 54, n6 = 55, n7 = 56, n8 = 57, n9 = 58, Subtract = 59,
			Add = 60, Decimal = 61, Multiply = 62, Divide = 63, F1 = 64, F2 = 65, F3 = 66, F4 = 67, F5 = 68, F6 = 69, F7 = 70,
			F8 = 71, F9 = 72, F10 = 73, F11 = 74, F12 = 75, F13 = 76, F14 = 77, F15 = 78, F16 = 79, F17 = 80, F18 = 81, F19 = 82,
			F20 = 83, F21 = 84, F22 = 85, F23 = 86, F24 = 87, Up = 88, Down = 89, Left = 90, Right = 91, Page_up = 92, Page_down = 93,
			Home = 94, End = 95, Insert = 96, Delete = 97, Scroll_lock = 98, Num_lock = 99, Caps_lock = 100,
			Tab = 101, Enter = 102, Backspace = 103, Esc = 104,
			Plus = 105, Shift = 106, Ctrl = 107
		);

		DEnum(MouseAxis, u32, Undefined = 0, X = 1, Y = 2, Mouse_wheel = 3);
		DEnum(MouseButton, u32, Undefined = 0, Left = 1, Middle = 2, Right = 3, Back = 4, Forward = 5);

	}

}