#pragma once
#include "array.h"

namespace oi {

	template<typename T>
	class List : public Array<T> {

	public:

		List() = default;
		~List() = default;
		List(const size_t count) : Array<T>(count), objects(count) {}
		List(const size_t count, const T *dat) : Array<T>(count, dat), objects(count) {}
		List(const size_t count, const T &def) : Array<T>(count, def), objects(count) {}
		List(const T *begin, const T *end) : Array<T>(begin, end), objects(this->count) {}
		List(const std::initializer_list<T> &val) : Array<T>(val), objects(this->count) {}
		List(const Array<T> &arr) : Array<T>(arr), objects(this->count) {}
		List(Array<T> &&arr) : Array<T>(arr), objects(this->count) {}

		template<size_t tcount>
		List(const T(&dat)[tcount]) : Array<T>(dat), objects(tcount) {
			this->data = new T[tcount]();
			TCopyArray<T>::exec(this->data, dat, tcount);
		}

		List(const List &arr) : Array<T>(arr), objects(objects) { }
		List(List &&arr) : Array<T>(arr), objects(arr.objects) { }

		List &operator=(const List &arr);
		List &operator=(List &&arr);
		List &operator=(const Array<T> &arr);
		List &operator=(Array<T> &&arr);

		//Makes sure there's x objects available
		List &reserve(const size_t objects);

		//Gets rid of all unused data
		List &shrinkToFit();

		//Resizes the container to hold x objects
		List &resize(const size_t objects);

		Array<T> toArray();

		List &operator+=(const List &arr);
		List operator+(const List &arr) const;

		List &pushBack(const T &val);
		List &pushBack(const List<T> &val);
		List &pushFront(const T &val);
		List &pushFront(const List<T> &val);

		List &popBack(const size_t count = 1);
		List &popFront(const size_t count = 1);

		List &insert(const T &val, size_t pos);
		List &insert(const List<T> &val, const size_t pos);
		List &erase(const size_t pos, const size_t count = 1);
		List &eraseValue(const T &val);

		List &clear();

		size_t find(const T &val);

		T *last() { return pickIfTrue(this->data, this->data + objects - 1, objects != 0); }
		T *end() { return this->data + objects; }

		const T *last() const { return pickIfTrue(this->data, this->data + objects - 1, objects != 0); }
		const T *end() const { return this->data + objects; }

		size_t lastIndex() const { return (objects - 1) * (objects > 0); }

		size_t size() const { return objects; }
		size_t dataSize() const { return objects * sizeof(T); }
		bool empty() const { return objects != 0; }

	private:

		size_t objects{};

	};

	template<typename T>
	List<T> &List<T>::operator=(const List &arr) {
		Array<T>::operator=(arr);
		objects = arr.objects;
		return *this;
	}

	template<typename T>
	List<T> &List<T>::operator=(List &&arr) {
		Array<T>::operator=(arr);
		objects = arr.objects;
		return *this;
	}

	template<typename T>
	List<T> &List<T>::operator=(const Array<T> &arr) {
		Array<T>::operator=(arr);
		objects = arr.size();
		return *this;
	}

	template<typename T>
	List<T> &List<T>::operator=(Array<T> &&arr) {
		Array<T>::operator=(arr);
		objects = arr.size();
		return *this;
	}

	template<typename T>
	List<T> &List<T>::operator+=(const List &arr) {

		if (objects + arr.objects > this->count) {			//Resize if no space left
			Array<T> target(objects + arr.objects);
			TMoveArray<T>::exec(target.begin(), this->data, objects);
			TCopyArray<T>::exec(target.begin(), arr.data, arr.objects, objects);
			Array<T>::operator=(std::move(target));
		} else											//Resize and copy
			TCopyArray<T>::exec(this->data + objects, arr.data, arr.objects);

		objects += arr.objects;
		return *this;
	}

	template<typename T>
	List<T> List<T>::operator+(const List &arr) const {
		Array<T> target(objects + arr.objects);
		TCopyArray<T>::exec(target.begin(), this->data, objects);
		TCopyArray<T>::exec(target.begin() + objects, arr.data, arr.objects);
		return target;
	}

	template<typename T>
	size_t List<T>::find(const T &val) {

		for (size_t i = 0; i < objects; ++i)
			if (this->data[i] == val)
				return i;

		return objects;
	}

	template<typename T>
	List<T> &List<T>::reserve(const size_t objs) {

		if (objs > this->count) {
			Array<T> resized(objs);
			TMoveArray<T>::exec(resized.begin(), this->data, objects);
			Array<T>::operator=(std::move(resized));
		}

		return *this;
	}

	template<typename T>
	List<T> &List<T>::resize(const size_t objs) {

		if (objs != this->count) {
			Array<T> resized(objs);
			TMoveArray<T>::exec(resized.begin(), this->data, pickIfTrue(objects, objs, objs < objects));
			objects = objs;
			Array<T>::operator=(std::move(resized));
		}

		return *this;
	}

	template<typename T>
	Array<T> List<T>::toArray() {
		Array<T> resized(objects);
		TCopyArray<T>::exec(resized.begin(), this->data, objects);
		return resized;
	}

	template<typename T>
	List<T> &List<T>::clear() {

		if constexpr (!std::is_pod<T>::value && !std::is_arithmetic<T>::value)
			for (size_t i = 0; i < size(); ++i)
				this->data[i] = T();

		objects = 0;
		return *this;
	}

	template<typename T>
	List<T> &List<T>::shrinkToFit() {

		if (objects == this->count)
			return *this;

		Array<T> elements(objects);
		TMoveArray<T>::exec(elements.begin(), this->data, objects);
		Array<T>::operator=(std::move(elements));
		return *this;
	}

	template<typename T>
	List<T> &List<T>::eraseValue(const T &value) {
		return erase(find(value));
	}

	template<typename T>
	List<T> &List<T>::popFront(const size_t tcount) {
		return erase(0, tcount);
	}

	template<typename T>
	List<T> &List<T>::popBack(const size_t tcount) {
		return erase(pickIfTrue(size() - tcount, tcount < size()), tcount);
	}

	template<typename T>
	List<T> &List<T>::erase(const size_t pos, const size_t tcount) {

		if (pos >= size())
			return *this;

		if (pos == 0 && tcount >= objects)
			return clear();

		if(pos + tcount < size())
			TMoveArray<T>::exec(this->data + pos, this->data + pos + tcount, size() - pos - tcount);

		objects -= tcount;

		if constexpr (!std::is_pod<T>::value && !std::is_arithmetic<T>::value)
			for (size_t i = objects; i < objects + tcount; ++i)
				this->data[i] = T();

		return *this;
	}

	template<typename T>
	List<T> &List<T>::pushFront(const T &t) {
		return insert(t, 0);
	}

	template<typename T>
	List<T> &List<T>::pushBack(const T &t) {
		return insert(t, size());
	}

	template<typename T>
	List<T> &List<T>::insert(const T &val, size_t pos) {

		pos = pickIfTrue(pos, size(), pos > size());
		bool shouldExpand = objects + 1 > this->count;

		if (shouldExpand) {

			Array<T> copy(this->count * 2);

			if(pos != 0)
				TMoveArray<T>::exec(copy.begin(), this->data, pos);

			TCopyArray<T>::exec(copy.begin(), &val, 1, pos);

			if (pos != size())
				TMoveArray<T>::exec(copy.begin() + pos + 1, this->data + pos, size() - pos);

			Array<T>::operator=(std::move(copy));

		} else {

			if (pos != size()) {
				Array<T> result(this->count);

				if (pos != 0)
					TMoveArray<T>::exec(result.begin(), this->data, pos);

				TCopyArray<T>::exec(result.begin() + pos, &val, 1);
				TMoveArray<T>::exec(result.begin() + pos + 1, this->data + pos, size() - pos);
				Array<T>::operator=(std::move(result));
				++objects;
				return *this;
			}

			TCopyArray<T>::exec(this->data + pos, &val, 1);

		}

		++objects;
		return *this;
	}

	/*List &pushBack(const List<T> &val);
	List &pushFront(const List<T> &val);
	List &insert(const List<T> &val, const size_t pos);*/

}