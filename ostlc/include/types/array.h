#pragma once
#include "template/templatefuncs.h"

namespace oi {

	///Statically sized array

	template<typename T, size_t count = 0>
	class Array {

	public:

		Array(const T *dat);
		Array(const T &def);
		Array() : Array(T{}) {}

		Array(const T (&dat)[count]) {
			TCopyArray<T>::exec(data, dat, count);
		}

		template<typename ...args>
		Array(const T &t0, const args&... arg) {
			TFillArray<T, args...>::exec(data, 0, count, t0, arg...);
		}

		Array(const Array &arr);
		Array(Array &&arr) = delete;

		Array &operator=(const Array &arr);
		Array &operator=(Array &&arr) = delete;

		T *begin() { return data; }
		T *last() { return data + count - 1; }
		T *end() { return data + count; }

		size_t lastIndex() const { return (count - 1) * (count > 0); }

		T &operator[](size_t i) { return data[i]; }

		void set(size_t i, const T &t) { data[i] = t; }

		const T *begin() const { return data; }
		const T *last() const { return data + count - 1; }
		const T *end() const { return data + count; }

		const T &operator[](size_t i) const { return data[i]; }

		const T &get(size_t i) const { return data[i]; }

		size_t size() const { return count; }
		size_t dataSize() const { return count * sizeof(T); }

	protected:

		T data[count] = {};

	};

	///Dynamically sized array

	template<typename T>
	class Array<T, 0> {

	public:

		Array() {}
		~Array();
		Array(size_t count);
		Array(size_t count, const T *dat);
		Array(size_t count, const T &def);
		Array(const T *begin, const T *end);

		template<size_t _count>
		Array(const T(&dat)[_count]) : count(_count) {
			data = new T[count]();
			TCopyArray<T>::exec(data, dat, count);
		}

		Array(std::initializer_list<T> val): count(val.end() - val.begin()) {
			data = new T[count]();
			TCopyArray<T>::exec(data, (T*) val.begin(), count);
		}

		Array(const Array &arr);
		Array(Array &&arr);

		Array &operator=(const Array &arr);
		Array &operator=(Array &&arr);

		T *begin() { return data; }
		T *last() { return count == 0 ? data : data + count - 1; }
		T *end() { return data + count; }

		Array pushFront(const T &t) const {
			Array copy(count + 1);
			TCopyArray<T>::exec(copy.data + 1, data, count);
			copy[0] = t;
			return copy;
		}

		Array pushBack(const T &t) const {
			Array copy(count + 1);
			TCopyArray<T>::exec(copy.data, data, count);
			*copy.last() = t;
			return copy;
		}

		T &operator[](size_t i) { return data[i]; }
		const T &operator[](size_t i) const { return data[i]; }

		void set(size_t i, const T &t) { data[i] = t; }

		const T *begin() const { return data; }
		const T *last() const { return count == 0 ? data : data + count - 1; }
		const T *end() const { return data + count; }

		size_t lastIndex() const { return (count - 1) * (count > 0); }

		const T &get(size_t i) const { return data[i]; }

		size_t size() const { return count; }
		size_t dataSize() const { return count * sizeof(T); }
		bool empty() const { return count != 0; }

	protected:

		T *data = nullptr;
		size_t count = 0;

	};

	///Dynamically sized array implementations

	template<typename T>
	Array<T>::~Array() {
		if (data)
			delete[] data;
	}

	template<typename T>
	Array<T>::Array(size_t count) : count(count) {
		if (count)
			data = new T[count]();
	}

	template<typename T>
	Array<T>::Array(size_t count, const T *dat) : count(count) {
		if (count) {
			data = new T[count]();
			TCopyArray<T>::exec(data, dat, count);
		}
	}

	template<typename T>
	Array<T>::Array(const Array &arr) : count(arr.count) {
		if (count) {
			data = new T[count]();
			TCopyArray<T>::exec(data, arr.data, count);
		}
	}

	template<typename T>
	Array<T>::Array(Array &&arr) : data(arr.data), count(arr.count) {
		arr.data = nullptr;
		arr.count = 0;
	}

	template<typename T>
	Array<T> &Array<T>::operator=(const Array &arr) {

		count = arr.count;

		if (count) {
			data = new T[count]();
			TCopyArray<T>::exec(data, arr.data, count);
		}

		return *this;
	}

	template<typename T>
	Array<T> &Array<T>::operator=(Array &&arr) {
		data = arr.data;
		count = arr.count;
		arr.data = nullptr;
		arr.count = 0;
		return *this;
	}

	template<typename T>
	Array<T>::Array(size_t count, const T &def) : count(count) {
		if (count) {

			data = new T[count]();

			for (size_t i = 0; i < count; ++i)
				data[i] = def;

		}
	}

	template<typename T>
	Array<T>::Array(const T *begin, const T *end) {

		count = (end - begin);

		if (count) {
			
			data = new T[count]();

			for (size_t i = 0; i < count; ++i)
				data[i] = begin[i];
		}
	}

	///Statically sized array implementations

	template<typename T, size_t count>
	Array<T, count>::Array(const T *dat) {
		TCopyArray<T>::exec(data, dat, count);
	}

	template<typename T, size_t count>
	Array<T, count>::Array(const Array &arr) {
		TCopyArray<T>::exec(data, arr.data, count);
	}

	template<typename T, size_t count>
	Array<T, count> &Array<T, count>::operator=(const Array &arr) {
		TCopyArray<T>::exec(data, arr.data, count);
		return *this;
	}

	template<typename T, size_t count>
	Array<T, count>::Array(const T &t) {
		for (size_t i = 0; i < count; ++i)
			data[i] = t;
	}

}