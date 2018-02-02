#pragma once

#include <Template/Enum.h>
#include <Template/PlatformDefines.h>

namespace oi {

	namespace wc {

		#ifdef __WINDOWS__
		DEnum(Click, u32, Undefined = 0, Left = 1, Middle = 2, Right = 3, Back = 4, Forward = 5);
		#endif

	}

}