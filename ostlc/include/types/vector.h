#pragma once

#include "template/templatefuncs.h"
#include "types/generic.h"
#include "utils/log.h"
#include "template/common.h"
#include <cmath>
#include <limits>

namespace oi {

	//Data for a nth-vector (where n != 2 && != 3 && != 4)
	template<typename T, u32 n>
	struct TVecStorage {
		T arr[n];
	};

	//Data for 2-vector
	template<typename T>
	struct TVecStorage<T, 2> {

		union {
			T arr[2];
			struct {
				T x, y;
			};
		};

	};

	//Data for 3-vector (and functions meant for Vec3 only)
	template<typename T>
	struct TVecStorage<T, 3> {

		union {
			T arr[3];
			struct {
				T x, y, z;
			};
		};

	};

	//Data for 4-vector
	template<typename T>
	struct TVecStorage<T, 4> {

		union {
			T arr[4];
			struct {
				T x, y, z, w;
			};
		};

	};

	struct VecFloatHelper {

		static bool isInvalid(f32 f) {
			u32 asint = (*(u32*)&f);
			return (asint >= 0x7F800000U /* +Infinity & +NaN*/ && asint <= 0x80000000U /* -0 */) || asint >= 0xFF800000U /* -Infinity & -NaN */;
		}

		static bool isInvalid(f64 f) {
			u64 asint = (*(u64*)&f);
			return (asint >= 0x7FF0000000000000U /* +Infinity & +NaN*/ && asint <= 0x8000000000000000U /* -0 */) || asint >= 0xFFF0000000000000U /* -Infinity & -NaN */;
		}

	};

	template<typename T, u32 n>
	class TVec : public TVecStorage<T, n> {
	
	public:

		TVec() : TVec((T)0) {}

		template<typename T2>
		TVec(T2 t) {
			for (u32 i = 0; i < n; ++i)
				this->arr[i] = (T) t;
		}

		template<typename T2, u32 n2>
		TVec(const TVec<T2, n2> &other) : TVec() {
			for (u32 i = 0; i < n2 && i < n; ++i)
				this->arr[i] = (T) other.arr[i];
		}
		
		template<typename T2, u32 n2, typename ...args>
		TVec(const TVec<T2, n2> &other, args... arg) : TVec() {

			for (u32 i = 0; i < n2 && i < n; ++i)
				this->arr[i] = (T) other.arr[i];

			u32 start = n2;

			FillArray::run_s(this->arr, n2 < n ? n2 : n, arg...);
		}

		template<typename T2, typename ...args>
		TVec(T2 t, args... arg): TVec() {
			FillArray::run_s(this->arr, 0, t, arg...);
		}

		TVec &operator+=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				this->arr[i] += other.arr[i];

			return *this;
		}

		TVec &operator-=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				this->arr[i] -= other.arr[i];

			return *this;
		}

		TVec &operator/=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				this->arr[i] /= other.arr[i];

			return *this;
		}

		TVec &operator*=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				this->arr[i] *= other.arr[i];

			return *this;
		}

		TVec operator+(const TVec &other) const {
			TVec res = *this;
			res += other;
			return res;
		}

		TVec operator-(const TVec &other) const {
			TVec res = *this;
			res -= other;
			return res;
		}

		TVec operator/(const TVec &other) const {
			TVec res = *this;
			res /= other;
			return res;
		}

		TVec operator*(const TVec &other) const {
			TVec res = *this;
			res *= other;
			return res;
		}

		TVec &operator=(const TVec &other) {
			for (u32 i = 0; i < n; ++i)
				this->arr[i] = other.arr[i];
			return *this;
		}

		TVec(const TVec &other) {
			for (u32 i = 0; i < n; ++i)
				this->arr[i] = other.arr[i];
		}

		TVec operator-() const {
			TVec res = *this;
			for (u32 i = 0; i < n; ++i)
				res.arr[i] = -this->arr[i];
			return res;
		}

		T magnitudeSquared() const {

			T res = 0;

			for (u32 i = 0; i < n; ++i)
				res += this->arr[i] * this->arr[i];

			return res;
		}

		T magnitude() const {
			return (T) std::sqrt(magnitudeSquared());
		}

		T &operator[](u32 i) { return this->arr[i]; }
		T get(u32 i) const { return this->arr[i]; }

		TVec normalize() const {
			return *this / magnitude();
		}

		TVec sqrt() const {

			TVec res = *this;

			for (u32 i = 0; i < n; ++i)
				res.arr[i] = (T)std::sqrt(res.arr[i]);

			return res;
		}

		TVec clamp(T mi, T ma) const {
			return min(ma).max(mi);
		}

		TVec min(T x) const {

			TVec res = *this;

			for (u32 i = 0; i < n; ++i)
				res.arr[i] = this->arr[i] < x ? this->arr[i] : x;

			return res;
		}

		TVec max(T x) const {

			TVec res = *this;

			for (u32 i = 0; i < n; ++i)
				res.arr[i] = this->arr[i] > x ? this->arr[i] : x;

			return res;
		}

		TVec min(const TVec &other) const {

			TVec res = *this;

			for (u32 i = 0; i < n; ++i)
				if (other.arr[i] < res.arr[i])
					res.arr[i] = other.arr[i];

			return res;
		}

		TVec max(const TVec &other) const {

			TVec res = *this;

			for (u32 i = 0; i < n; ++i)
				if (other.arr[i] > res.arr[i])
					res.arr[i] = other.arr[i];

			return res;
		}

		bool operator==(const TVec &other) const {

			for (u32 i = 0; i < n; ++i)
				if (this->arr[i] != other.arr[i])
					return false;

			return true;
		}

		bool operator!=(const TVec &other) const {
			return !(*this == other);
		}

		TVec inverse() const {

			TVec res = *this;
			for (u32 i = 0; i < n; ++i)
				res.arr[i] = (T)1 / this->arr[i];

			return res;
		}

		static constexpr TVec zero() {
			return TVec();
		}

		static constexpr TVec one() {
			return TVec((T)1);
		}

		static constexpr TVec min() {
			return TVec(std::numeric_limits<T>::min());
		}

		static constexpr TVec max() {
			return TVec(std::numeric_limits<T>::max());
		}

		template<typename T2, u32 n2>
		operator TVec<T2, n2>() const {

			TVec<T2, n2> conv;

			for (u32 i = 0; i < n2 && i < n; ++i)
				conv.arr[i] = (T2) this->arr[i];

			return conv;
		}

		//Only implemented for TVec<f<x>. n>
		//Lerps between this and other with the percentage given (0 = this, 1 = other)
		//Vec3(92, 95, 99).lerp(Vec3(28, 39, 25), 0.5)
		//Will return 92 * 0.5 + 28 * 0.5, 95 * 0.5 + 39 * 0.5, 99 * 0.5 + 25 * 0.5
		//(60, 67, 62)
		template<typename T2>
		TVec lerp(const TVec &other, const TVec<T2, n> &perc) const {

			static_assert(std::is_floating_point<T2>::value, "TVec<T, n>::lerp<T2>; T2 should be a floating point (f<x>)");

			TVec<T2, n> f0 = *this;
			TVec<T2, n> f1 = other;

			return TVec(f0 * perc + f1 * (TVec<T2, n>(1) - perc));
		}

		TVec floor() const {

			static_assert(std::is_floating_point<T>::value, "TVec<T,n>::floor can only be performed on a floating point vector");

			TVec result;

			for (u32 i = 0; i < n; ++i)
				result.arr[i] = (T) std::floor(this->arr[i]);

			return result;
		}

		TVec fract() const {

			static_assert(std::is_floating_point<T>::value, "TVec<T,n>::fract can only be performed on a floating point vector");

			return *this - floor();
		}

		TVec round() const {

			static_assert(std::is_floating_point<T>::value, "TVec<T,n>::round can only be performed on a floating point vector");

			TVec result;

			for (u32 i = 0; i < n; ++i)
				result.arr[i] = (T) std::round(this->arr[i]);

			return result;
		}

		TVec ceil() const {

			static_assert(std::is_floating_point<T>::value, "TVec<T,n>::ceil can only be performed on a floating point vector");

			TVec result;

			for (u32 i = 0; i < n; ++i)
				result.arr[i] = (T) std::ceil(this->arr[i]);

			return result;
		}

		TVec &operator%=(const TVec &v) {
			return *this = operator%(v);
		}

		TVec operator%(const TVec &v) const {

			static_assert(std::is_floating_point<T>::value, "TVec<T,n>::mod (operator%) can only be performed on a floating point vector");

			TVec result;

			for (u32 i = 0; i < n; ++i)
				result.arr[i] = this->arr[i] / v.arr[i];

			return result.fract() * v;
		}

		TVec reflect(const TVec &normal) const {

			static_assert(std::is_floating_point<T>::value && n == 3, "TVec<T,n>::reflect can only be performed on a floating point Vec3");

			return *this - normal * (T) 2 * dot(normal);
		}

		T dot(const TVec &v) const {

			static_assert(std::is_floating_point<T>::value && n >= 2, "TVec<T,n>::dot can only be performed on a floating point Vec2 and above");

			T result = 0;

			for (u32 i = 0; i < n; ++i)
				result += this->arr[i] * v.arr[i];

			return result;
		}

		TVec cross(const TVec &other) {

			static_assert(std::is_floating_point<T>::value && n == 3, "TVec<T,n>::dot can only be performed on a floating point Vec3");
			
			return { this->y * other.z - this->z * other.y, this->z * other.x - this->x * other.z, this->x * other.y - this->y * other.x };
		}

		//Only implemented for TVec<u32, n>
		//Uses other vector as dimensions.
		//Example where n = 3:
		//Vec3u(1, 2, 3).getIndex(Vec3u(5, 5, 5));
		//Will return 1 % 5 + 2 % 5 * 5 + 3 % 5 * 5 * 5 = 1 + 10 + 75 = 86
		T getIndex(const TVec &other) {

			static_assert(std::is_integral<T>::value , "TVec<T,n>::getIndex can only be performed on an integer vector");

			T res = 0;

			for (u32 i = 0; i < n; ++i) {

				if (other.arr[i] == 0) return 0;

				T y = this->arr[i] % other.arr[i];

				for (u32 j = 0; j < i; ++j)
					y *= other.arr[j];

				res += y;
			}

			return res;
		}

		//Only implemented for TVec where T is integer
		//Uses 'this' as dimensions.
		//Example where n = 3:
		//Vec3(5, 5, 5).fromIndex(86)
		//Will return 86 % 5 = 1, 86 / 5 % 5 = 2, 86 / 5 / 5 % 5 = 3
		//(1, 2, 3)
		TVec fromIndex(const u32 &other) {

			TVec res(other);

			for (u32 i = 0; i < n; ++i)
				if (this->arr[i] != 0) {

					for (u32 j = 0; j < i; ++j)
						res.arr[i] /= this->arr[j];

					res.arr[i] %= this->arr[i];
				}
				else
					return {};

			return res;
		}

		//Only implemented for floating points;
		//Sets invalid floating point values (like signed zero, (+/-)infinity, (+/-)NaN) to 0.
		TVec &fix() {

			static_assert(std::is_floating_point<T>::value, "TVec<T,n>::fix can only be performed on a floating point vector");

			for (u32 i = 0; i < n; ++i)
				if (VecFloatHelper::isInvalid(this->arr[i]))
					this->arr[i] = 0;

			return *this;

		}

		//Retrieves values and puts them into a different order
		//Example for TVec3:
		//query(1, 0, 2) would be something like '.yxz' in GLSL/HLSL
		template<typename ...args>
		TVec<T, ParamCount<args...>::get> query(args... arg) const {
			TVec<T, ParamCount<args...>::get> vec;
			CopyArray::run(this->arr, vec.getArray(), 0, arg...);
			return vec;
		}

		//Sets values in a different order
		//Example for TVec3:
		//replace(Vec4u(1, 2, 3, 4), 2, 1)
		//Would be the same as
		//v3[2] = 1
		//v3[1] = 2
		//Aka; xy = Vec4u(1, 2, 3, 4).zy
		template<u32 n2, typename ...args>
		TVec &replace(const TVec<T, n2> &other, args... arg) {
			CopyArray::run(const_cast<TVec<T, n2>*>(&other)->getArray(), this->arr, 0, arg...);
			return *this;
		}

		//Get the aspect between x and y (x / y)
		//Only works for TVec2
		T getAspect() const {
			static_assert(n == 2, "TVec<T, n>::getAspect is only implemented for TVec2");
			return (T) this->x / this->y;
		}

		constexpr u32 getLength() const { return n; }
		constexpr u32 getSize() const { return n * (u32) sizeof(T); }

		typedef T (&ArrayType)[n];

		ArrayType getArray() { return this->arr; }
	};


	template<class T, u32 n>
	String::String(TVec<T, n> t) {

		static_assert(std::is_arithmetic<T>::value, "T is not a number");

		std::stringstream ss;

		for (u32 i = 0; i < n; ++i)
			ss << t[i] << (i != n - 1 ? ", " : "");

		source = ss.str();
	}

	template<typename T, u32 n>
	TVec<T, n> String::toVector() const {

		static_assert(std::is_arithmetic<T>::value, "T is not a number");

		TVec<T, n> result;

		std::vector<String> s = contains(',') ? split(',') : split(' ');

		u32 i = 0;
		for (auto &e : s) {

			if(i < n)
				result[i] = (T) e.trim().toFloat();

			++i;
		}

		return result;
	}

	template<typename T>
	using TVec2 = TVec<T, 2>;

	template<typename T>
	using TVec3 = TVec<T, 3>;

	template<typename T>
	using TVec4 = TVec<T, 4>;

	typedef TVec2<f32> Vec2;
	typedef TVec3<f32> Vec3;
	typedef TVec4<f32> Vec4;

	typedef TVec2<u32> Vec2u;
	typedef TVec3<u32> Vec3u;
	typedef TVec4<u32> Vec4u;

	typedef TVec2<u16> Vec2us;
	typedef TVec3<u16> Vec3us;
	typedef TVec4<u16> Vec4us;

	typedef TVec2<u8> Vec2ub;
	typedef TVec3<u8> Vec3ub;
	typedef TVec4<u8> Vec4ub;

	typedef TVec2<i32> Vec2i;
	typedef TVec3<i32> Vec3i;
	typedef TVec4<i32> Vec4i;

	typedef TVec2<i16> Vec2is;
	typedef TVec3<i16> Vec3is;
	typedef TVec4<i16> Vec4is;

	typedef TVec2<i8> Vec2ib;
	typedef TVec3<i8> Vec3ib;
	typedef TVec4<i8> Vec4ib;

	typedef TVec2<f64> Vec2d;
	typedef TVec3<f64> Vec3d;
	typedef TVec4<f64> Vec4d;

	typedef TVec2<gbool> Vec2b;
	typedef TVec3<gbool> Vec3b;
	typedef TVec4<gbool> Vec4b;
}