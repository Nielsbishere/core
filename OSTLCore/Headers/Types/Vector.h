#pragma once

#include "Template/TemplateFuncs.h"
#include "Types/Generic.h"
#include "Utils/Log.h"

#define VEC_SPECIAL_MAKE(name, type, errorId, errorCode)				\
template<typename T>													\
static TVec<T, n> name(const TVec<T,n> &other) {						\
	Log::throwError<TVec<T, n>, errorId>(##errorCode);					\
	return TVec<T, n>(0);												\
}																		\
static TVec<type, n> name(const TVec<type, n> &other);

#define VEC_SPECIAL_DEFINE(name, type)								\
template<u32 n>															\
TVec<type, n> TVecSpecial<n>::##name(const TVec<type, n> &v)			\

namespace oi {

	template<class T, u32 n> class TVec;

	//Special functions for TVec
	template<u32 n>
	struct TVecSpecial {

		template<class T>
		static u32 getIndex(const TVec<T, n> &v, const TVec<T, n> &other){
			Log::throwError<TVec<T, n>, 0x0>("TVec<T,n>::asIndex is a function that is only implemented for TVec<u32,n>");
			return 0;
		}

		static u32 getIndex(const TVec<u32, n> &self, const TVec<u32, n> &other);

		template<class T>
		static TVec<T, n> fromIndex(const TVec<T, n> &v, const u32 &other) {
			Log::throwError<TVec<T, n>, 0x1>("TVec<T,n>::fromIndex is a function that is only implemented for TVec<u32,n>");
			return 0;
		}

		static TVec<u32, n> fromIndex(const TVec<u32, n> &self, const u32 &other);

		VEC_SPECIAL_MAKE(floor, f32, 0x7, "TVec<T,n>::floor is a function that is only implemented for TVec<f32,n>");
		VEC_SPECIAL_MAKE(fract, f32, 0x8, "TVec<T,n>::fract is a function that is only implemented for TVec<f32,n>");
		VEC_SPECIAL_MAKE(ceil, f32, 0x9, "TVec<T,n>::ceil is a function that is only implemented for TVec<f32,n>");
		VEC_SPECIAL_MAKE(round, f32, 0xA, "TVec<T,n>::round is a function that is only implemented for TVec<f32,n>");

		template<typename T>
		static TVec<T, n> mod(const TVec<T,n> &v, const TVec<T, n> &other) {
			Log::throwError<TVec<T, n>, 0xB>("TVec<T,n>::mod is a function that is only implemented for TVec<f32,n>");
			return TVec<T, n>(0);
		}

		static TVec<f32, n> mod(const TVec<f32, n> &v, const TVec<f32, n> &other);
	};

	template<typename T>
	struct TVecSpecial2 {

		template<u32 n>
		static TVec<T, n> cross(const TVec<T, n> &v, const TVec<T, n> &other) {
			Log::throwError<TVec<T, n>, 0xC>("TVec<T,n>::cross is a function that is only implemented for TVec<T,3>");
			return TVec<T, n>(0);
		}

		static TVec<T, 3> cross(const TVec<T, 3> &v, const TVec<T, 3> &other);
	};


	template<class T, u32 n>
	class TVec {

	public:

		TVec() : TVec((T)0) {}

		TVec(T t) {
			for (u32 i = 0; i < n; ++i)
				arr[i] = t;
		}

		template<typename T2, u32 n2>
		TVec(const TVec<T2, n2> &other) : TVec() {
			for (u32 i = 0; i < n2 && i < n; ++i)
				arr[i] = (T)other.get(i);
		}
		
		template<typename T2, u32 n2, typename ...args>
		TVec(const TVec<T2, n2> &other, args... arg) : TVec() {

			for (u32 i = 0; i < n2 && i < n; ++i)
				arr[i] = (T)other.get(i);

			u32 start = n2;

			FillArray::run_s(arr, n2 < n ? n2 : n, arg...);
		}

		template<typename ...args>
		TVec(T t, args... arg): TVec() {
			FillArray::run_s(arr, 0, t, arg...);
		}

		TVec &operator+=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				arr[i] += other.arr[i];

			return *this;
		}

		TVec &operator-=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				arr[i] -= other.arr[i];

			return *this;
		}

		TVec &operator/=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				arr[i] /= other.arr[i];

			return *this;
		}

		TVec &operator*=(const TVec &other) {

			for (u32 i = 0; i < n; ++i)
				arr[i] *= other.arr[i];

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
				arr[i] = other.arr[i];
			return *this;
		}

		TVec(const TVec &other) {
			for (u32 i = 0; i < n; ++i)
				arr[i] = other.arr[i];
		}

		TVec operator-() const {
			TVec res = *this;
			for (u32 i = 0; i < n; ++i)
				res.arr[i] = -arr[i];
			return res;
		}

		T magnitudeSquared() const {
			T res = 0;
			for (u32 i = 0; i < n; ++i)
				res += arr[i] * arr[i];

			return res;
		}

		T magnitude() const {
			return (T)std::sqrt(magnitudeSquared());
		}

		T &operator[](u32 i) { return arr[i]; }
		T get(u32 i) const { return arr[i]; }

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
				res.arr[i] = arr[i] < x ? arr[i] : x;

			return res;
		}

		TVec max(T x) const {
			TVec res = *this;
			for (u32 i = 0; i < n; ++i)
				res.arr[i] = arr[i] > x ? arr[i] : x;

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
				if (arr[i] != other.arr[i])
					return false;
			return true;
		}

		bool operator!=(const TVec &other) const {
			return !(*this == other);
		}

		TVec inverse() const {
			TVec res = *this;
			for (u32 i = 0; i < n; ++i)
				res.arr[i] = (T)1 / arr[i];
			return res;
		}

		static TVec zero() {
			return TVec();
		}

		static TVec one() {
			return TVec((T)1);
		}

		static TVec min() {
			return TVec(std::numeric_limits<T>::min());
		}

		static TVec max() {
			return TVec(std::numeric_limits<T>::max());
		}

		template<typename T2, u32 n2>
		operator TVec<T2, n2>() const {
			TVec<T2, n2> conv;
			for (u32 i = 0; i < n2 && i < n; ++i)
				conv.arr[i] = (T2)arr[i];
			return conv;
		}

		//Only implemented for TVec<u32, n>
		//Uses other vector as dimensions.
		//Example where n = 3:
		//Vec3u(1, 2, 3).asIndex(Vec3u(5, 5, 5));
		//Will return 1 % 5 + 2 % 5 * 5 + 3 % 5 * 5 * 5 = 1 + 10 + 75 = 86
		u32 asIndex(const TVec &other) const {
			return TVecSpecial<n>::getIndex(*this, other);
		}

		//Only implemented for TVec<u32, n>
		//Uses this as dimensions.
		//Example where n = 3:
		//Vec3(5, 5, 5).fromIndex(86)
		//Will return 86 % 5 = 1, 86 / 5 % 5 = 2, 86 / 5 / 5 % 5 = 3
		//(1, 2, 3)
		TVec fromIndex(const u32 &other) const {
			return TVecSpecial<n>::fromIndex(*this, other);
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

			return TVec(f0 * perc + f1 * (TVec<T2, n>(1.0) - perc));
		}

		T &x() {
			if (n < 1) Log::throwError<TVec<T, n>, 0x3>("TVec<T,n>::x is a variable that only exists for n >= 1");
			return arr[0];
		}

		T &y() {
			if (n < 2) Log::throwError<TVec<T, n>, 0x4>("TVec<T,n>::x is a variable that only exists for n >= 2");
			return arr[1];
		}

		T &z() {
			if (n < 3) Log::throwError<TVec<T, n>, 0x5>("TVec<T,n>::x is a variable that only exists for n >= 3");
			return arr[2];
		}

		T &w() {
			if (n < 4) Log::throwError<TVec<T, n>, 0x6>("TVec<T,n>::x is a variable that only exists for n >= 4");
			return arr[3];
		}

		TVec floor() const {
			return TVecSpecial<n>::floor(*this);
		}

		TVec fract() const {
			return TVecSpecial<n>::fract(*this);
		}

		TVec round() const {
			return TVecSpecial<n>::round(*this);
		}

		TVec ceil() const {
			return TVecSpecial<n>::ceil(*this);
		}

		TVec &operator%=(const TVec &v) {
			*this = TVecSpecial<n>::mod(*this, v);
			return *this;
		}

		TVec operator%(const TVec &v) const {
			return TVecSpecial<n>::mod(*this, v);
		}

		TVec reflect(const TVec &normal) const {
			return *this - normal * 2 * dot(normal);
		}

		T dot(const TVec &v) const {
			T result = 0;
			for (u32 i = 0; i < n; ++i)
				result += arr[i] * v.arr[i];
			return result;
		}

		//Only implemented for TVec3
		TVec cross(const TVec &v) const {
			return TVecSpecial2<T>::cross(*this, v);
		}

		//Retrieves values and puts them into a different order
		//Example for TVec3:
		//query(1, 0, 2) would be something like '.yxz' in GLSL/HLSL
		template<typename ...args>
		TVec<T, ParamSize<args...>::getArguments> query(args... arg) const {
			TVec<T, ParamSize<args...>::getArguments> vec;
			CopyArray::run(arr, vec.getArray(), 0, arg...);
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
			CopyArray::run(const_cast<TVec<T, n2>*>(&other)->getArray(), arr, 0, arg...);
			return *this;
		}

		const u32 getLength() const { return n; }
		const u32 getSize() const { return n * sizeof(T); }

		typedef T (&ArrayType)[n];

		ArrayType getArray() { return arr; }

	protected:

		T arr[n];
	};

	template<u32 n>
	u32 TVecSpecial<n>::getIndex(const TVec<u32, n> &v, const TVec<u32, n> &other) {
		u32 res = 0;
		for (u32 i = 0; i < n; ++i) {
			if (other.get(i) == 0) return 0;
			u32 y = v.get(i) % other.get(i);
			for (u32 j = 0; j < i; ++j)
				y *= other.get(j);
			res += y;
		}
		return res;
	}

	template<u32 n>
	TVec<u32, n> TVecSpecial<n>::fromIndex(const TVec<u32, n> &v, const u32 &other) {

		TVec<u32, n> res(other);

		for (u32 i = 0; i < n; ++i)
			if (v.get(i) != 0) {
				for (u32 j = 0; j < i; ++j)
					res[i] /= v.get(j);
				res[i] %= v.get(i);
			}
			else
				return TVec<u32, n>();

		return res;
	}

	VEC_SPECIAL_DEFINE(floor, f32) {
		TVec<f32, n> result = v;
		for (u32 i = 0; i < n; ++i)
			result[i] = (f32)std::floor(result[i]);
		return result;
	}

	VEC_SPECIAL_DEFINE(round, f32) {
		TVec<f32, n> result = v;
		for (u32 i = 0; i < n; ++i)
			result[i] = (f32)std::round(result[i]);
		return result;
	}

	VEC_SPECIAL_DEFINE(ceil, f32) {
		TVec<f32, n> result = v;
		for (u32 i = 0; i < n; ++i)
			result[i] = (f32)std::ceil(result[i]);
		return result;
	}

	VEC_SPECIAL_DEFINE(fract, f32) {
		TVec<f32, n> result = v;
		for (u32 i = 0; i < n; ++i)
			result[i] = result[i] - (f32)std::floor(result[i]);
		return result;
	}

	template<u32 n>
	TVec<f32, n> TVecSpecial<n>::mod(const TVec<f32, n> &v, const TVec<f32, n> &other) {
		return (v / other).fract() * other;
	}

	template<typename T>
	TVec<T, 3> TVecSpecial2<T>::cross(const TVec<T, 3> &v, const TVec<T, 3> &other) {
		return TVec<T, 3>(v.get(1) * other.get(2) - v.get(2) * other.get(1), v.get(2) * other.get(0) - v.get(0) * other.get(2), v.get(0) * other.get(1) - v.get(1) * other.get(0));
	}

	template<class T, u32 n>
	OString OString::fromVector(TVec<T, n> t) {

		static_assert(std::is_arithmetic<T>::value, "T is not a number");

		std::stringstream ss;

		for (u32 i = 0; i < n; ++i)
			ss << t[i] << (i != n - 1 ? ", " : "");

		return ss.str();
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

	typedef TVec2<i32> Vec2i;
	typedef TVec3<i32> Vec3i;
	typedef TVec4<i32> Vec4i;

	typedef TVec2<f64> Vec2d;
	typedef TVec3<f64> Vec3d;
	typedef TVec4<f64> Vec4d;

	typedef TVec2<gbool> Vec2b;
	typedef TVec3<gbool> Vec3b;
	typedef TVec4<gbool> Vec4b;
}