#pragma once

#include <algorithm>
#include "vector.h"
#include "generic.h"

namespace oi {

	template<typename T, u32 d = 2>
	class TGrid {

	public:

		TGrid(): arr(nullptr), len(0) { }

		TGrid(T t, TVec<u32, d> v, bool clearFlag = true) : size(v) {
			arr = new T[len = (size - 1).asIndex(size) + 1];

			if (clearFlag)
				std::fill(arr, arr + len, t);
		}

		T &operator[](u32 i) { return arr[i]; }
		T &operator[](TVec<u32, d> i) { return arr[i.asIndex(size)]; }

		const T &get(u32 i) const { return arr[i]; }
		const T &get(TVec<u32, d> i) const { return arr[i.asIndex(size)]; }

		TGrid(const TGrid &other) {
			copy(other);
		}

		TGrid &operator=(const TGrid &other) {
			return copy(other);
		}

		TGrid &operator=(TGrid &&other) {
			arr = other.arr;
			size = other.size;
			len = other.len;
			other.arr = nullptr;
			return *this;
		}

		~TGrid() {
			if(arr != nullptr)
				delete[] arr;
		}

		u32 getSize() { return len; }

		u32 getDimension(u32 i) const { return size.get(i);}
		TVec<u32, d> getDimensions() const { return size; }

	protected:

		TGrid &copy(const TGrid &other) {
			len = other.len;
			size = other.size;
			arr = new T[len];
			std::copy(other.arr, other.arr + len, arr);
			return *this;
		}

		T *arr;
		TVec<u32, d> size;
		u32 len;
	};

	typedef TGrid<f32> Grid2D;
	typedef TGrid<u32> Grid2Du;
	typedef TGrid<i32> Grid2Di;
	typedef TGrid<f64> Grid2Dd;

	typedef TGrid<f32, 3> Grid3D;
	typedef TGrid<u32, 3> Grid3Du;
	typedef TGrid<i32, 3> Grid3Di;
	typedef TGrid<f64, 3> Grid3Dd;

}