#pragma once

#include "Vector.h"

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
			memset(this, 0, sizeof(T) * w * h);
			makeScale(TVec < T, (w < h ? w : h) >(1));
		}

		TMatrix(const TMatrix &other) {
			memcpy(this, &other, sizeof(T) * w * h);
		}

		TMatrix &makeScale(TVec < T, (w < h) ? w : h > scale) {
			for (u32 i = 0; i < (w < h ? w : h); ++i)
				m[i][i] = scale[i];
			return *this;
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