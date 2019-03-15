#pragma once
#include "vector.h"

namespace oi {

	template<typename T>
	struct TQuat {

	private:

		static constexpr T halfDegToRad = T(3.141592653598 / 360);

	public:

		union {

			T data[4];

			TVec4<T> xyzw;

			struct {
				T x, y, z, w;
			};

			struct {
				TVec3<T> xyz;
				T _w;
			};

		};

		TQuat(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }
		TQuat(TVec4<T> xyzw) : xyzw(xyzw) {}
		TQuat(TVec3<T> xyz, T w) : xyz(xyz), _w(w) {}
		TQuat() : TQuat(0, 0, 0, 0) {}
		TQuat(const TQuat &other) : xyzw(other.xyzw) {}
		TQuat &operator=(const TQuat &other) { xyzw = other.xyzw; return *this; }

		TQuat operator*(const TQuat &b) const {

			return TQuat(
				x * b.w + w * b.x + y * b.z - z * b.y,
				y * b.w + w * b.y + z * b.x - x * b.z,
				z * b.w + w * b.z + x * b.y - y * b.x,
				w * b.w - x * b.x - y * b.y - z * b.z
			);

		}

		TQuat &operator*=(const TQuat &other) {
			return *this = (*this * other);
		}

		T length() const {
			return xyzw.magnitude();
		}

		TQuat normalize() const {
			return xyzw.normalize();
		}

		TQuat inverse() const {
			return inverseRotation().normalize();
		}

		TQuat inverseRotation() const {
			return TQuat(-x, -y, -z, w);
		}

		TQuat slerp(const TQuat &b, T t) const {

			static_assert(std::is_floating_point<T>::value, "TQuat::slerp is only available with floating point operations");

			const TVec4<T> &self = xyzw;
			TVec4<T> other = b.xyzw;

			T angle = self.dot(other);

			if (angle < 0) {	//-angle = angle
				other *= -1;
				angle *= -1;
			}

			if (angle >= 1)		//self == other or self == -other
				return *this;

			T halfTheta = std::acos(angle);
			T sinHalfTheta = std::sqrt(1 - angle * angle);

			if (sinHalfTheta < T(0.001))		//angle = 180 deg, so 50/50
				return self.lerp(other, T(0.5));

			T ratioA = std::sin((1 - t) * halfTheta) / sinHalfTheta;
			T ratioB = std::sin(t * halfTheta) / sinHalfTheta;

			return self * ratioA + other * ratioB;

		}

		static TQuat fromPosition(TVec3<T> pos) {
			return TQuat(pos, 0);
		}

		static TQuat rotateAxis(TVec3<T> axis, f32 angleDeg) {
			f32 fi = angleDeg * halfDegToRad;
			return TQuat(axis * std::sin(fi), std::cos(fi));
		}

		static TQuat rotate(TVec3<T> eulerDeg) {

			TVec3<T> euler = eulerDeg * halfDegToRad;

			const TVec3<T> c = euler.cos();
			const TVec3<T> s = euler.sin();

			const T cycp = c.yaw * c.pitch;
			const T sycp = s.yaw * c.pitch;
			const T cysp = c.yaw * s.pitch;
			const T sysp = s.yaw * s.pitch;

			const T cr = c.z, sr = s.z;

			return TQuat(
				sycp * cr + cysp * sr,
				cysp * cr - sycp * sr,
				cycp * sr + sysp * cr,
				cycp * cr - sysp * sr
			);

		}

		static constexpr TQuat identity() { return { 0, 0, 0, 1 }; }

		operator String() {
			return xyzw;
		}

		bool operator==(const TQuat &other) const {
			return xyzw == other.xyzw;
		}

		T delta(const TQuat &other) const {
			return (xyzw - other.xyzw).magnitude();
		}

	};

	typedef TQuat<f32> Quat;
	typedef TQuat<f64> Quatd;
	typedef TQuat<u32> Quatu;
	typedef TQuat<i32> Quati;

}