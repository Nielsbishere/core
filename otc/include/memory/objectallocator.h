#pragma once
#include "types/bitset.h"

namespace oi {

	//Dynamic ObjectAllocator

	//Similar to T[] with Bitset
	template<typename T>
	class ObjectAllocator {

	public:

		static constexpr u32 objSize = (u32) sizeof(T);

		//ObjectAllocator handles allocation
		ObjectAllocator(u32 size): occ(size), buf(size * objSize) {
			for (u32 i = 0; i < size; ++i)
				::new (buf.addr() + i * objSize) T();
		}

		//ObjectAllocator doesn't handle allocation
		ObjectAllocator(u32 size, Buffer buf) : buf(buf), occ(size) { 
			if (buf.size() != size * objSize) 
				Log::throwError<ObjectAllocator, 0x0>("ObjectAllocator buffer should match size * sizeof(T)"); 
		}

		~ObjectAllocator() {
			if (owned) buf.deconstruct();
		}

		ObjectAllocator(const ObjectAllocator &other) {
			copy(other);
		}

		ObjectAllocator &operator=(const ObjectAllocator &other) {
			copy(other);
			return *this;
		}

		bool isOccupied(u32 i) const {
			return i > occ.getBits() || occ[i];
		}

		T &operator[](u32 i) {
			return buf.operator[]<T>(i * objSize);
		}

		const T &operator[](u32 i) const {
			return buf.operator[]<T>(i * objSize);
		}

		u32 allocate(const T &t) {

			u32 j = occ.getBits();

			for (u32 i = 0; i < j; ++i)
				if (!occ[i]) {
					buf.operator[]<T>(i * objSize) = t;
					occ[i] = true;
					return i;
				}

			return j;
		}

		T *alloc(const T &t) {

			u32 j = occ.getBits();

			for (u32 i = 0; i < j; ++i)
				if (!occ[i]) {
					buf.operator[]<T>(i * objSize) = t;
					occ[i] = true;
					j = i;
					break;
				}

			return (T*)(buf.addr() + j * objSize);

		}

		bool deallocate(u32 i) {

			if (i >= occ.getBits() || !occ[i])
				return Log::error("Invalid dealloc; out of range or not allocated");

			occ[i] = false;
			buf.operator[]<T>(i * objSize).~T();
			return true;
		}

		bool dealloc(T *t) {

			if (t < (T*)buf.addr() || t >= (T*)(buf.addr() + buf.size()))
				return Log::error("Invalid dealloc; out of range or not allocated");

			u32 i = u32(t - (T*)buf.addr());

			occ[i] = false;
			buf.operator[]<T>(i * objSize).~T();
			return true;
		}

		Buffer getBuffer() { return buf; }

		u32 find(T *t) {

			if (t < (T*)buf.addr() || t >= (T*)(buf.addr() + buf.size())) {
				Log::error("Invalid find; out of range or not allocated");
				return occ.getBits();
			}

			return u32(t - (T*)buf.addr());

		}

	protected:

		void copy(const ObjectAllocator &other) {

			occ = other.occ;

			if ((owned = other.owned)) {

				buf = Buffer(other.buf.size());

				for (u32 i = 0, j = other.occ.getBits(); i < j; ++i)
					if (occ[i])
						::new (buf.addr() + i * objSize) T(other.buf.template operator[]<T>(i * objSize));
					else
						::new (buf.addr() + i * objSize) T();

			} else
				buf = other.buf;
		}

	private:

		Buffer buf;
		Bitset occ;
		bool owned = true;

	};

	//Static ObjectAllocator

	//Similar to T[n] with Bitset(n)
	template<typename T, u32 n>
	class StaticObjectAllocator {

	public:

		static constexpr u32 objSize = (u32) sizeof(T);
		static constexpr u32 size = n * objSize;

		//ObjectAllocator handles allocation
		StaticObjectAllocator() {
			for (u32 i = 0; i < n; ++i)
				::new (data + i) T();
		}

		StaticObjectAllocator(const StaticObjectAllocator &other) {
			copy(other);
		}

		StaticObjectAllocator &operator=(const StaticObjectAllocator &other) {
			copy(other);
			return *this;
		}

		bool isOccupied(u32 i) const {
			return i > n || occ[i];
		}

		T &operator[](u32 i) {
			return data[i];
		}

		const T &operator[](u32 i) const {
			return data[i];
		}

		u32 allocate(const T &t) {

			for (u32 i = 0; i < n; ++i)
				if (!occ[i]) {
					data[i] = t;
					occ[i] = true;
					return i;
				}

			return n;
		}

		T *alloc(const T &t) {

			u32 j = n;

			for (u32 i = 0; i < n; ++i)
				if (!occ[i]) {
					data[i] = t;
					occ[i] = true;
					j = i;
					break;
				}

			return data + j;

		}

		bool deallocate(u32 i) {

			if (i >= n || !occ[i])
				return Log::error("Invalid dealloc; out of range or not allocated");

			occ[i] = false;
			data[i].~T();
			return true;
		}

		bool dealloc(T *t) {

			if (t < data || t >= data + n)
				return Log::error("Invalid dealloc; out of range or not allocated");

			u32 i = u32(t - data);

			occ[i] = false;
			data[i].~T();
			return true;
		}

		u32 find(T *t){

			if (t < data || t >= data + n) {
				Log::error("Invalid find; out of range or not allocated");
				return n;
			}

			return u32(t - data);

		}

		Buffer toBuffer() { return Buffer::construct((u8*)data, size); }
		CopyBuffer toCb() { return CopyBuffer(size, (u8*)data); }

		typedef T (&ArrayType)[n];

		ArrayType toArray() { return data; }

	protected:

		void copy(const StaticObjectAllocator &other) {

			occ = other.occ;

			for (u32 i = 0; i < n; ++i)
				if (occ[i])
					::new (data + i) T(other.data[i]);
				else
					::new (data + i) T();
		}

	private:

		T data[n];
		StaticBitset<n> occ;

	};

}