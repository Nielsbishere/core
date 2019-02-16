#pragma once
#include "template/templatefuncs.h"

namespace oi {

	///Statically sized array

	template<typename T, size_t count = 0>
	class Array {

	public:

		Array(T *dat);
		Array(const T &def);

		Array(const T(&dat)[count]) {
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

		T &operator[](size_t i) { return data[i]; }

		void set(size_t i, const T &t) { data[i] = t; }

		const T *begin() const { return data; }
		const T *last() const { return data + count - 1; }
		const T *end() const { return data + count; }

		const T &operator[](size_t i) const { return data[i]; }

		const T &get(size_t i) const { return data[i]; }

		size_t size() const { return count; }
		size_t dataSize() const { return count * sizeof(T); }

	private:

		T data[count] = {};

	};

	///Dynamically sized array

	template<typename T>
	class Array<T, 0> {

	public:

		Array() {}
		~Array();
		Array(size_t count);
		Array(size_t count, T *dat);
		Array(size_t count, const T &def);

		template<size_t _count>
		Array(const T(&dat)[_count]) : count(_count) {
			data = new T[count]();
			TCopyArray<T>::exec(data, dat, count);
		}

		template<typename ...args>
		Array(const T &t0, const T &t1, const args&... arg) : count(2 + sizeof...(args)) {
			data = new T[count]();
			TFillArray<T, T, args...>::exec(data, 0, -1, t0, t1, arg...);
		}

		Array(const Array &arr);
		Array(Array &&arr);

		Array &operator=(const Array &arr);
		Array &operator=(Array &&arr);

		T *begin() { return data; }
		T *last() { return data + count - 1; }
		T *end() { return data + count; }

		T &operator[](size_t i) { return data[i]; }

		void set(size_t i, const T &t) { data[i] = t; }

		const T *begin() const { return data; }
		const T *last() const { return data + count - 1; }
		const T *end() const { return data + count; }

		const T &operator[](size_t i) const { return data[i]; }

		const T &get(size_t i) const { return data[i]; }

		size_t size() const { return count; }
		size_t dataSize() const { return count * sizeof(T); }
		bool empty() const { return count != 0; }

	private:

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
		data = new T[count]();
	}

	template<typename T>
	Array<T>::Array(size_t count, T *dat) : count(count) {
		data = new T[count]();
		TCopyArray<T>::exec(data, dat, count);
	}

	template<typename T>
	Array<T>::Array(const Array &arr) : count(arr.count) {
		data = new T[count]();
		TCopyArray<T>::exec(data, arr.data, count);
	}

	template<typename T>
	Array<T>::Array(Array &&arr) : data(arr.data), count(arr.count) {
		arr.data = nullptr;
		arr.count = 0;
	}

	template<typename T>
	Array<T> &Array<T>::operator=(const Array &arr) {
		data = new T[count = arr.count]();
		TCopyArray<T>::exec(data, arr.data, count);
	}

	template<typename T>
	Array<T> &Array<T>::operator=(Array &&arr) {
		data = arr.data;
		count = arr.count;
		arr.data = nullptr;
		arr.count = 0;
	}

	template<typename T>
	Array<T>::Array(size_t count, const T &def) : count(count) {

		data = new T[count]();

		for (size_t i = 0; i < count; ++i)
			data[i] = def;

	}

	///Statically sized array implementations

	template<typename T, size_t count>
	Array<T, count>::Array(T *dat) {
		TCopyArray<T>::exec(data, dat, count);
	}

	template<typename T, size_t count>
	Array<T, count>::Array(const Array &arr) {
		TCopyArray<T>::exec(data, arr.data, count);
	}

	template<typename T, size_t count>
	Array<T, count> &Array<T, count>::operator=(const Array &arr) {
		TCopyArray<T>::exec(data, arr.data, count);
	}

	template<typename T, size_t count>
	Array<T, count>::Array(const T &t) {
		for (size_t i = 0; i < count; ++i)
			data[i] = t;
	}

}