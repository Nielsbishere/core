#pragma once

#include <types/generic.h>

namespace oi {

	namespace wc {

		enum class WindowAction : u32 {
			NONE = 0U,
			IN_FOCUS = 1U << 0U,
			FULL_SCREEN = 1U << 1U
		};

	}

}