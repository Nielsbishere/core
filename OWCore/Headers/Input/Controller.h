#pragma once

#include <Template/Enum.h>

namespace oi {

	namespace wc {

		DEnum(ControllerButton, u32, Unknown = 0, Cross = 1, Square = 2, Triangle = 3, Circle = 4, Down = 5, Left = 6, Up = 7, Right = 8, L1 = 9, R1 = 10, L3 = 11, R3 = 12, Share = 13, Options = 14);
		DEnum(ControllerAxis, u32, Unknown = 0, Lx = 1, Ly = 2, Rx = 3, Ry = 4, L2 = 5, R2 = 6);

		struct Controller {
			const static u32 maxControllers = 4;
		};
	}

}