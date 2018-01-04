#pragma once

#include <functional>
#include "Types/Generic.h"

namespace oi {

	template<typename T, typename ...args>
	struct ParamSize {
		static constexpr u32 getArguments = 1U + ParamSize<args...>::getArguments;
	};

	template<typename T>
	struct ParamSize<T> {
		static constexpr u32 getArguments = 1U;
	};

	struct FillArray {

		template<typename T, u32 n, typename T2, typename ...args>
		static void run(T(&arr)[n], u32 i, T2 t, args... param) {
			if (i >= n) return;
			arr[i] = t;
			run(arr, ++i, param...);
		}

		template<typename T, u32 n, typename T2>
		static void run(T(&arr)[n], u32 i, T2 t) {
			if (i >= n) return;
			arr[i] = t;
		}

	};

	struct CopyArray {

		template<typename T, u32 n, u32 n2, typename T2, typename ...args>
		static void run(const T (&old)[n], T (&nw)[n2], u32 j, T2 t, args... param) {

			u32 i = (u32)t;
			if(i < n2 && i < n)
				nw[j] = old[i];

			run(old, nw, ++j, param...);
		}

		template<typename T, u32 n, u32 n2, typename T2>
		static void run(const T (&old)[n], T(&nw)[n2], u32 j, T2 t) {

			u32 i = (u32)t;
			if (i < n2 && i < n)
				nw[j] = old[i];
		}

	};

}