#pragma once

#include "Vector.h"
#include "Generic.h"

namespace oi {

	template<typename T>
	struct TextureHelper;

	template<typename T, u32 d = 2>
	class TGrid {

		friend struct TextureHelper<T>;

	public:

		TGrid(): arr(nullptr), len(0) { }

		TGrid(T t, TVec<u32, d> v, bool clearFlag = true) : size(v) {
			arr = new T[len = (size - 1).asIndex(size) + 1];

			if (clearFlag)
				for (u32 i = 0; i < len; ++i)
					memcpy(arr + i, &t, sizeof(T));
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
			memcpy(arr, other.arr, sizeof(T) * len);
			return *this;
		}

		T *arr;
		TVec<u32, d> size;
		u32 len;
	};

}