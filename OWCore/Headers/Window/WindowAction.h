#pragma once

#include <Types/Generic.h>

namespace oi {
	namespace wc {
		enum class WindowAction : u32 {
			NONE = 0,
			MOVE = 1,
			RESIZE = 2,
			IN_FOCUS = 4,

			SHOW = 4
		};
	}
}