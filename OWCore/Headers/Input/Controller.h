#pragma once

#include <Template/Enum.h>

namespace oi {

	namespace wc {

		UEnum(ControllerButton, "Unknown", "Cross", "Square", "Triangle", "Circle", "Down", "Left", "Up", "Right", "L1", "R1", "L3", "R3", "Share", "Options");
		UEnum(ControllerAxis, "Lx", "Ly", "Rx", "Ry", "L2", "R2");

		struct Controller {
			const static u32 maxControllers = 4;
		};
	}

}