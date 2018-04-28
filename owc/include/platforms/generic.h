#pragma once

#ifdef __WINDOWS__

#include "windows.h"

namespace oi {

	namespace wc {

		struct WNull {};
		
		#define WindowExt WWindow

		typedef WKey KeyExt;
		typedef WNull ControllerButtonExt;
		typedef void WindowHandleExt;

	}

}

#elif __ANDROID__

#include "android.h"

namespace oi {

	namespace wc {

		#define WindowExt AWindow

		typedef AKey KeyExt;
		typedef AControllerButton ControllerButtonExt;
		typedef android_app WindowHandleExt;

	}

}

#endif