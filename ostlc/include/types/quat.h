#pragma once
#include "vector.h"

namespace oi {

	template<typename T>
	struct TQuat {

		union {

			T data[4];

			struct {
				T x, y, z, w;
			};

		};

		TQuat(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }
		TQuat(TVec4<T> xyzw) : x(xyzw.x), y(xyzw.y), z(xyzw.z), w(xyzw.w) {}
		TQuat(TVec3<T> xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
		TQuat() : TQuat(0, 0, 0, 0) {}

		operator TVec4<T>() const { return TVec4<T>(x, y, z, w); }

		TVec4<T> &asVec4() { return *(TVec4<T>*) this; }
		const TVec4<T> &asVec4() const { return *(TVec4<T>*) this; }

		TQuat operator*(const TQuat &other) const {

			const TVec4<T> q0 = other.asVec4() * w;

			return TQuat(
				q0.x + x * other.w + y * other.z - z * other.y,
				q0.y - x * other.z + y * other.w + z * other.x,
				q0.z + x * other.y - y * other.x + z * other.w,
				q0.w - x * other.x - y * other.y - z * other.z
			);

		}

		TQuat &operator*=(const TQuat &other) {
			return *this = (*this * other);
		}

		T length() const {
			return (T)sqrt((f64)(x * x + y * y + z * z + w * w));
		}

		TQuat normalize() const {
			return *this / length();
		}

		TQuat inverse() const {
			return inverseRotation() / length();
		}

		TQuat inverseRotation() const {
			return TQuat(-x, -y, -z, w);
		}

		static TQuat fromPosition(TVec3<T> pos) {
			return TQuat(pos, 0);
		}

		static TQuat rotateAxis(TVec3<T> axis, f32 angleDeg) {
			angleDeg = angleDeg / 180.f * 3.141592653598f * 0.5f;
			return TQuat(axis * sin(angleDeg), cos(angleDeg));
		}

		static TQuat rotate(TVec3<T> eulerDeg) {

			eulerDeg = eulerDeg / 180.f * 3.141592653598f * 0.5f;

			const Vec3 c = eulerDeg.cos();
			const Vec3 s = eulerDeg.sin();

			const f32 cycp = c.y * c.x;
			const f32 sycp = s.y * c.x;
			const f32 cysp = c.y * s.x;
			const f32 sysp = s.y * s.x;

			return Quat(
				cycp * c.z + sysp * s.z,
				cycp * s.z + sysp * c.z,
				sycp * s.z + cysp * c.z,
				sycp * c.z + cysp * s.z
			);

		}

	};

	typedef TQuat<f32> Quat;
	typedef TQuat<f64> Quatd;
	typedef TQuat<u32> Quatu;
	typedef TQuat<i32> Quati;

}