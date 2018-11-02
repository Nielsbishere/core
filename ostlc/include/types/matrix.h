#pragma once

#include <algorithm>
#include <cstring>
#include "vector.h"

namespace oi {

	template<typename T, u32 w, u32 h>
	class MatrixData {

	public:

		MatrixData() {}
		union {
			T m[w][h];
			T f[w * h];
		};

	};

	template<typename T>
	class MatrixData<T, 2, 2> {

	public:

		MatrixData() {}
		union {
			T m[2][2];
			T f[4];

			struct {
				TVec2<T> xAxis;
				TVec2<T> yAxis;
			};
		};

	};

	template<typename T>
	class MatrixData<T, 3, 3> {

	public:

		MatrixData() {}
		union {
			T m[3][3];
			T f[9];

			struct {
				TVec2<T> xAxis;
				T wx;
				TVec2<T> yAxis;
				T wy;
				TVec2<T> translation;
				T one;
			};
		};

	};

	template<typename T>
	class MatrixData<T, 4, 4> {

	public:

		MatrixData() {}
		union {
			T m[4][4];
			T f[16];

			struct {
				TVec3<T> xAxis;
				T wx;
				TVec3<T> yAxis;
				T wy;
				TVec3<T> zAxis;
				T wz;
				TVec3<T> translation;
				T one;
			};
		};

	};



	template<typename T, u32 w, u32 h>
	class TMatrix : public MatrixData<T, w, h> {

	public:

		TMatrix() {
			memset(this->f, 0, sizeof(this->f));
			setScale(TVec<T, (w < h ? w : h)>(1));
		}

		TMatrix(T &t) {
			std::fill(this->f, this->f + w * h, t);
		}

		template<typename T2, u32 w2, u32 h2>
		TMatrix(const TMatrix<T2, w2, h2> other) {
			for (u32 i = 0; i < w && i < w2; ++i)
				for (u32 j = 0; j < h && j < h2; ++j)
					this->m[i][j] = (T) other.m[i][j];
		}

		TMatrix(const TMatrix &other) {
			copy(other);
		}

		TMatrix &operator=(const TMatrix &other) {
			copy(other);
			return *this;
		}

		TMatrix &setScale(TVec<T, (w < h) ? w : h > scale) {
			for (u32 i = 0; i < (w < h ? w : h); ++i)
				this->m[i][i] = scale[i];
			return *this;
		}

		TMatrix &operator+=(const TMatrix &other) {
			for (u32 i = 0; i < w*h; ++i)
				this->f[i] += other.f[i];
			return *this;
		}

		TMatrix &operator-=(const TMatrix &other) {
			for (u32 i = 0; i < w*h; ++i)
				this->f[i] -= other.f[i];
			return *this;
		}

		TMatrix &operator/=(const TMatrix &other) {
			for (u32 i = 0; i < w*h; ++i)
				this->f[i] /= other.f[i];
			return *this;
		}

		TMatrix operator+(const TMatrix &other) const {
			TMatrix copy = *this;
			return copy += other;
		}

		TMatrix operator-(const TMatrix &other) const {
			TMatrix copy = *this;
			return copy -= other;
		}

		TMatrix operator/(const TMatrix &other) const {
			TMatrix copy = *this;
			return copy /= other;
		}

		TVec<T, h> vertical(u32 i) const {

			TVec<T, h> v({});

			for (u32 j = 0; j < h; ++j)
				v[j] = this->m[i][j];

			return v;
		}

		TVec<T, w> horizontal(u32 j) const {

			TVec<T, w> v({});

			for (u32 i = 0; i < w; ++i)
				v[i] = this->m[i][j];

			return v;

		}

		TMatrix &operator*=(const TMatrix &other) {
			return *this = *this * other;
		}

		TMatrix operator*(const TMatrix &other) const {

			TMatrix m;

			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					m.m[i][j] = horizontal(j).dot(other.vertical(i));

			return m;
		}

		bool operator==(const TMatrix &other) const {
			return memcmp(this->f, other.f, sizeof(other.f)) == 0;
		}

		bool operator!=(const TMatrix &other) const {
			return memcmp(this->f, other.f, sizeof(other.f)) != 0;
		}

		T &operator[](u32 i) {
			return this->f[i];
		}

		T &operator[](Vec2u i) {
			return i.getIndex(Vec2u(w, h));
		}

		TMatrix &operator*(T t) {
			for (u32 i = 0; i < w*h; ++i)
				this->f[i] *= t;
			return *this;
		}
		
		static TMatrix zero() {
			static const TMatrix zer = { TVec<T, w>() };
			return zer;
		}

		static TMatrix makeScale(TVec<T, (w < h) ? w : h > v) {
			TMatrix mat;
			mat.setScale(v);
			return mat;
		}

		static TMatrix makeRotateX(T x) {

			static_assert((w == 4 || w == 3) && (h == 4 || h == 3), "TMatrix::makeRotate only works if there are orientation params (4x3, 4x4, 3x4 and 3x3)");

			TMatrix m;
			m.m[1][1] = m.m[2][2] = (T) cos(x);
			m.m[2][1] = (T) -sin(x);
			m.m[1][2] = (T) sin(x);

			return m;
		}

		static TMatrix makeRotateY(T y) {

			static_assert((w == 4 || w == 3) && (h == 4 || h == 3), "TMatrix::makeRotate only works if there are orientation params (4x3, 4x4, 3x4 and 3x3)");

			TMatrix m;
			m.m[0][0] = m.m[2][2] = (T) cos(y);
			m.m[0][2] = (T) -sin(y);
			m.m[2][0] = (T) sin(y);

			return m;
		}

		static TMatrix makeRotateZ(T z) {

			static_assert(w >= 2 && w <= 4 && h >= 2 && h <= 4, "TMatrix::makeRotateZ only works if there are 2d orientation params (2/3/4 x 2/3/4)");

			TMatrix m;
			m.m[0][0] = m.m[1][1] = (T) cos(z);
			m.m[1][0] = (T) -sin(z);
			m.m[0][1] = (T) sin(z);

			return m;
		}

		static TMatrix makeRotate(TVec3<T> dr) {

			static_assert((w == 4 || w == 3) && (h == 4 || h == 3), "TMatrix::makeRotate only works if there are orientation params (4x3, 4x4, 3x4 and 3x3)");

			dr = dr / 180.f * 3.1415926535f;	//Convert to rad

			return makeRotateX(dr.x) * makeRotateY(dr.y) * makeRotateZ(dr.z);
		}

		static TMatrix makeTranslate(TVec3<T> v) {

			static_assert((w == 3 || w == 4) && (h == 3 || h == 4), "TMatrix::makeTranslate only works if there are translation params (4x3, 4x4, 3x4 and 3x3)");

			TMatrix result;

			for (u32 j = 0; j < 3; ++j)
				result.m[w - 1][j] = v[j];

			return result;
		}

		static TMatrix makeModel(TVec3<T> pos, TVec3<T> drot, TVec3<T> scl) {

			static_assert(w == 4 && h == 4, "TMatrix::makeModel is only available on TMatrix4x4");

			return makeTranslate(pos) * makeRotate(drot) * makeScale(TVec4<T>(scl, (T) 1));
		}

		static TMatrix makeView(TVec3<T> eye, TVec3<T> center, TVec3<T> up) {

			static_assert(w == 4 && h == 4, "TMatrix::makeView is only available on TMatrix4x4");

			TVec3<T> z = (eye - center).normalize();
			TVec3<T> x = (up.cross(z)).normalize();
			TVec3<T> y = (z.cross(x)).normalize();

			TMatrix m = TMatrix().setOrientation(x, y, z);
			return m * makeTranslate(-eye);
		}

		TMatrix &setOrientation(TVec3<T> x, TVec3<T> y, TVec3<T> z) {

			static_assert((w == 4 || w == 3) && (h == 4 || h == 3), "TMatrix::makeRotate only works if there are orientation params (4x3, 4x4, 3x4 and 3x3)");

			TVec3<T> arr[3] = { x, y, z };

			for (u32 i = 0; i < 3; ++i)
				for (u32 j = 0; j < 3; ++j)
					this->m[i][j] = arr[j][i];

			return *this;
		}

		TMatrix<T, h, w> transpose() const {

			TMatrix<T, h, w> res;

			for (u32 i = 0; i < w; ++i)
				for (u32 j = 0; j < h; ++j)
					res.m[j][i] = this->m[i][j];

			return res;
		}

		static TMatrix makeOrtho(T l, T r, T b, T t, T n, T f) {

			f32 width = r - l;
			f32 height = t - b;

			TMatrix m;
			m.m[0][0] = 2 / width;
			m.m[1][1] = 2 / height;
			m.m[2][2] = 1 / (f - n);
			m.m[3][2] = -n / (f - n);

			return m;
		}

		static TMatrix makePerspective(T fov, T asp, T n, T f) {

			TMatrix m;

			T scale = (T)(1 / tan(fov * 0.5f / 180 * 3.1415926535f));

			m.m[0][0] = scale / asp;
			m.m[1][1] = scale;
			m.m[2][2] = -((f + n) / (f - n));
			m.m[3][3] = 0;
			m.m[2][3] = -1;
			m.m[3][2] = -(2 * f * n / (f - n));

			return m;
		}

	private:

		void copy(const TMatrix &other) {
			memcpy(this->f, other.f, sizeof(other.f));
		}

	};

	typedef TMatrix<f32, 4, 4> Matrix4;
	typedef TMatrix<f32, 4, 4> Matrix;
	typedef TMatrix<f32, 3, 3> Matrix3;
	typedef TMatrix<f32, 2, 2> Matrix2;
	typedef TMatrix<f32, 2, 3> Matrix2x3;
	typedef TMatrix<f32, 2, 4> Matrix2x4;
	typedef TMatrix<f32, 3, 2> Matrix3x2;
	typedef TMatrix<f32, 3, 4> Matrix3x4;
	typedef TMatrix<f32, 4, 3> Matrix4x3;
	typedef TMatrix<f32, 4, 2> Matrix4x2;
	
	typedef TMatrix<f64, 4, 4> Matrix4d;
	typedef TMatrix<f64, 3, 3> Matrix3d;
	typedef TMatrix<f64, 2, 2> Matrix2d;
	typedef TMatrix<f64, 2, 3> Matrix2x3d;
	typedef TMatrix<f64, 2, 4> Matrix2x4d;
	typedef TMatrix<f64, 3, 2> Matrix3x2d;
	typedef TMatrix<f64, 3, 4> Matrix3x4d;
	typedef TMatrix<f64, 4, 3> Matrix4x3d;
	typedef TMatrix<f64, 4, 2> Matrix4x2d;

	typedef TMatrix<i32, 4, 4> Matrix4i;
	typedef TMatrix<i32, 3, 3> Matrix3i;
	typedef TMatrix<i32, 2, 2> Matrix2i;
	typedef TMatrix<i32, 2, 3> Matrix2x3i;
	typedef TMatrix<i32, 2, 4> Matrix2x4i;
	typedef TMatrix<i32, 3, 2> Matrix3x2i;
	typedef TMatrix<i32, 3, 4> Matrix3x4i;
	typedef TMatrix<i32, 4, 3> Matrix4x3i;
	typedef TMatrix<i32, 4, 2> Matrix4x2i;

	typedef TMatrix<u32, 4, 4> Matrix4u;
	typedef TMatrix<u32, 3, 3> Matrix3u;
	typedef TMatrix<u32, 2, 2> Matrix2u;
	typedef TMatrix<u32, 2, 3> Matrix2x3u;
	typedef TMatrix<u32, 2, 4> Matrix2x4u;
	typedef TMatrix<u32, 3, 2> Matrix3x2u;
	typedef TMatrix<u32, 3, 4> Matrix3x4u;
	typedef TMatrix<u32, 4, 3> Matrix4x3u;
	typedef TMatrix<u32, 4, 2> Matrix4x2u;
}