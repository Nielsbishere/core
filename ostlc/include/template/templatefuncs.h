#pragma once

#include <functional>
#include "types/generic.h"

namespace oi {

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

		template<typename T, u32 n, typename T2, typename ...args>
		static void run_s(T(&arr)[n], u32 i, T2 t, args... param) {
			if (i >= n) return;
			arr[i] = (T)t;
			run_s(arr, ++i, param...);
		}

		template<typename T, u32 n, typename T2>
		static void run_s(T(&arr)[n], u32 i, T2 t) {
			if (i >= n) return;
			arr[i] = (T)t;
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

	template<typename T>
	inline static bool isSet(T enum0, T enum1) {
		return ((u32)enum0 & (u32)enum1) != 0;
	}

	//Filling an array

	template<typename T, typename ...args>
	struct TFillArray {

		template<typename T2>
		static inline void exec(T2 *dat, size_t i, size_t j, const T &t0, const args&... arg) {

			if (i == j)
				return;

			dat[i] = t0;
			TFillArray<args...>::exec(dat, i + 1, j, arg...);
		}

	};

	template<typename T>
	struct TFillArray<T> {

		template<typename T2>
		static inline void exec(T2 *dat, size_t i, size_t j, const T &t0) {
			if (i != j)
				dat[i] = t0;
		}

	};

	//Copying an array

	template<typename T, bool b = std::is_arithmetic<T>::value || std::is_pod<T>::value>
	struct TCopyArray {

		static inline void exec(T *dst, const T *src, size_t count, size_t offset = 0) {

			if (count == offset)
				return;

			dst[offset] = src[offset];
			exec(dst, src, count, offset + 1);

		}

	};

	template<typename T>
	struct TCopyArray<T, true> {

		static inline void exec(T *dst, const T *src, size_t count) {
			memcpy(dst, src, count * sizeof(T));
		}

	};

}