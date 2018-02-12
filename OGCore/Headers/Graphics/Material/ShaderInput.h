#pragma once

#include <Template/Enum.h>
#include <Types/GDataType.h>

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

			static GDataType getType(ShaderInputType type) {

				if (type.getIndex() >= 39 && type.getIndex() <= 104)
					return GDataType::oi_sampler;

				if (type.getIndex() >= 21)
					return GDataType::oi_undefined;

				return type.getValue().value;
			}

			static ShaderInputType fromType(GDataType type) {
				if (type.getIndex() <= GDataType(GDataType::oi_sampler).getIndex()) return type.getIndex();
				return ShaderInputType::Undefined;
			}

			//Whether or not the variable is allowed as an input for a shader
			static bool isShaderInput(ShaderInputType type) {
				GDataType base = getType(type).getValue().derivedId;
				return base == GDataType::oi_uint || base == GDataType::oi_int || base == GDataType::oi_float || base == GDataType::oi_double;
			}

		};

	}

}