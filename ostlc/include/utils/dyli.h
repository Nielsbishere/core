#pragma once

#include "types/string.h"

namespace oi {

	//Dynamic lib
	//.dll on Windows and .so on Unix
	class DyLi {

	public:

		DyLi(String path) : path(path) { init(path); }
		~DyLi();

		String getPath() const { return path; }
		bool isValid() const;

		//Get a function from the DyLiLoader
		//T is the function type; so bool (*SomeName)(bool, bool) would be a func that returns a bool from two bools
		//String name is the name of the function; the function must however be mentioned to be external
		template<typename T>
		T getFunc(String name) const {
			return (T) getFuncPtr(name);
		}

	protected:

		void init(String path);
		void *getFuncPtr(String name) const;

		String path;
		u8 platformData[1024];
	};

}