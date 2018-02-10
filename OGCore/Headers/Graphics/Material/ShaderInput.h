#pragma once

#include <Template/Enum.h>

namespace oi {

	namespace gc {

		DEnum(ShaderInputType, u32,
			Undefined = 0,
			Float = 1, Float2 = 2, Float3 = 3, Float4 = 4,
			Double = 5, Double2 = 6, Double3 = 7, Double4 = 8,
			Int = 9, Int2 = 10, Int3 = 11, Int4 = 12,
			Uint = 13, Uint2 = 14, Uint3 = 15, Uint4 = 16,
			Bool = 17, Bool2 = 18, Bool3 = 19, Bool4 = 20,

			Mat2x2 = 21, Mat3x3 = 22, Mat4x4 = 23, Mat2x3 = 24, Mat2x4 = 25, Mat3x2 = 26, Mat3x4 = 27, Mat4x3 = 28, Mat4x2 = 29,
			Mat2x2d = 30, Mat3x3d = 31, Mat4x4d = 32, Mat2x3d = 33, Mat2x4d = 34, Mat3x2d = 35, Mat3x4d = 36, Mat4x3d = 37, Mat4x2d = 38,

			Sampler1D = 39, Sampler2D = 40, Sampler3D = 41, SamplerCube = 42, Sampler1DShadow = 43, Sampler2DShadow = 44,
			Sampler1DArr = 45, Sampler2DArr = 46, Sampler1DShadowArr = 47, Sampler2DShadowArr = 48,
			Sampler2DMS = 49, Sampler2DMSArr = 50, SamplerCubeShadow = 51, SamplerBuffer = 52, Sampler2DRect = 53, Sampler2DRectShadow = 54,

			Sampler1Di = 55, Sampler2Di = 56, Sampler3Di = 57, SamplerCubei = 58, Sampler1DiArr = 59, Sampler2DiArr = 60,
			Sampler2DMSi = 61, Sampler2DMSiArr = 62, SamplerBufferi = 63, Sampler2DRecti = 64,

			Sampler1Du = 65, Sampler2Du = 66, Sampler3Du = 67, SamplerCubeu = 68, Sampler1DuArr = 69, Sampler2DuArr = 70,
			Sampler2DMSu = 71, Sampler2DMSuArr = 72, SamplerBufferu = 73, Sampler2DRectu = 74,

			Image1D = 75, Image2D = 76, Image3D = 77, Image2DRect = 78, ImageCube = 79, ImageBuffer = 80,
			Image1DArr = 81, Image2DArr = 82, Image2DMS = 83, Image2DMSArr = 84,

			Image1Di = 85, Image2Di = 86, Image3Di = 87, Image2DRecti = 88, ImageCubei = 89, ImageBufferi = 90,
			Image1DiArr = 91, Image2DiArr = 92, Image2DMSi = 93, Image2DMSiArr = 94,

			Image1Du = 95, Image2Du = 96, Image3Du = 97, Image2DRectu = 98, ImageCubeu = 99, ImageBufferu = 100,
			Image1DuArr = 101, Image2DuArr = 102, Image2DMSu = 103, Image2DMSuArr = 104,

			aUint = 105
		);

		struct ShaderInputHelper {

			//Returns Undefined when it is not of default types
			static ShaderInputType getBase(ShaderInputType t) {
				u32 i = t.getIndex();
				if (i == 0) return ShaderInputType();
				--i;
				if (i < 4) return ShaderInputType(1);		//Float[x]
				if (i < 8) return ShaderInputType(5);		//Double[x]
				if (i < 12) return ShaderInputType(9);		//Int[x]
				if (i < 16) return ShaderInputType(13);		//Uint[x]
				if (i < 20) return ShaderInputType(17);		//Bool[x]
				if (i < 29) return ShaderInputType(1);		//Float[x][y]
				if (i < 38) return ShaderInputType(5);		//Double[x][y]
				return ShaderInputType();
			}

			static u32 getSize(ShaderInputType type) {

				if (type.getIndex() >= 39 && type.getIndex() <= 104)
					return 8;

				ShaderInputType_s base = getBase(type);
				return getCount(type) * (base == ShaderInputType::Double ? 8 : 4);
			}

			//Whether or not the variable is allowed as an input for a shader
			static bool isShaderInput(ShaderInputType type) {
				ShaderInputType_s base = getBase(type);
				return base == ShaderInputType::Uint || base == ShaderInputType::Int || base == ShaderInputType::Float || base == ShaderInputType::Double;
			}

			//Returns 0 when it is not of default types
			static u32 getCount(ShaderInputType t) {
				u32 i = t.getIndex();
				if (i == 0) return 0;
				--i;
				if (i < 4) return i + 1;				//Float[x]
				if (i < 8) return i - 3;				//Double[x]
				if (i < 12) return i - 7;				//Int[x]
				if (i < 16) return i - 11;				//Uint[x]
				if (i < 20) return i - 15;				//Bool[x]
				if (i == 20 || i == 29) return 4;		//mat[2][2]
				if (i == 21 || i == 30) return 9;		//mat[3][3]	  
				if (i == 22 || i == 31) return 16;		//mat[4][4]
				if (i == 23 || i == 32) return 6;		//mat[2][3]
				if (i == 24 || i == 33) return 8;		//mat[2][4]
				if (i == 25 || i == 34) return 6;		//mat[3][2]
				if (i == 26 || i == 35) return 12;		//mat[3][4]
				if (i == 27 || i == 36) return 12;		//mat[4][3]
				if (i == 28 || i == 37) return 8;		//mat[4][2]
				return 0;
			}

		};

	}

}