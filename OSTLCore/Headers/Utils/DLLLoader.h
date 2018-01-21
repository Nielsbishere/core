#pragma once

#include "Template/PlatformDefines.h"
#include "Types/OString.h"
#include "Log.h"

namespace oi {

	#ifdef __WINDOWS__

	class DLLLoader {

	public:

		DLLLoader(OString _path) : path(_path) {
			inst = LoadLibrary(path.c_str());

			if (!inst)
				Log::throwError<DLLLoader, 0x0>(OString("Couldn't load DLL with path \"") + path + "\"");
		}

		OString getPath() { return path; }
		bool isValid() { return inst != NULL; }

		//Get a function from the DLLLoader
		//T is the function type; so bool (*SomeName)(bool, bool) would be a func that returns a bool from two bools
		//OString name is the name of the function; the function must however be mentioned to be external
		template<typename T>
		T getFunc(OString name) {
			return (T)GetProcAddress(inst, name.c_str());
		}

	private:

		OString path;
		HINSTANCE inst;
	};

	#endif

}