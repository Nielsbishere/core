#pragma once

#include <types/generic.h>

namespace oi {

	namespace wc {

		enum class WindowAction : u32 {
			NONE = 0U,
			MOVE = 1U << 0U,
			RESIZE = 1U << 1U,
			IN_FOCUS = 1U << 2U,
		};

	}

}