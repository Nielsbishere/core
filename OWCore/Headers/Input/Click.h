#pragma once

#include <Template/Enum.h>
#include <Template/PlatformDefines.h>

namespace oi {

	namespace wc {

		#ifdef __WINDOWS__
		UEnum(Click, "Undefined", "Left", "Middle", "Right", "Back", "Forward");
		#endif

	}

}