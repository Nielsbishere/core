#pragma once
#include <Types/OString.h>
#include <Template/Enum.h>
#include "API/OpenGL.h"

namespace oi {

	namespace gc {

		UEnum(ShaderInputType, 
			"Undefined",
			"Float", GL_FLOAT, "Float2", GL_FLOAT_VEC2, "Float3", GL_FLOAT_VEC3, "Float4", GL_FLOAT_VEC4,
			"Double", GL_DOUBLE, "Double2", GL_DOUBLE_VEC2, "Double3", GL_DOUBLE_VEC3, "Double4", GL_DOUBLE_VEC4,
			"Int", GL_INT, "Int2", GL_INT_VEC2, "Int3", GL_INT_VEC3, "Int4", GL_INT_VEC4,
			"Uint", GL_UNSIGNED_INT, "Uint2", GL_UNSIGNED_INT_VEC2, "Uint3", GL_UNSIGNED_INT_VEC3, "Uint4", GL_UNSIGNED_INT_VEC4,
			"Bool", GL_BOOL, "Bool2", GL_BOOL_VEC2, "Bool3", GL_BOOL_VEC3, "Bool4", GL_BOOL_VEC4,

			"Mat2x2", GL_FLOAT_MAT2, "Mat3x3", GL_FLOAT_MAT3, "Mat4x4", GL_FLOAT_MAT4, "Mat2x3", GL_FLOAT_MAT2x3, "Mat2x4", GL_FLOAT_MAT2x4, "Mat3x2", GL_FLOAT_MAT3x2, "Mat3x4", GL_FLOAT_MAT3x4, "Mat4x3", GL_FLOAT_MAT4x3, "Mat4x2", GL_FLOAT_MAT4x2,
			"Mat2x2d", GL_DOUBLE_MAT2, "Mat3x3d", GL_DOUBLE_MAT3, "Mat4x4d", GL_DOUBLE_MAT4, "Mat2x3d", GL_DOUBLE_MAT2x3, "Mat2x4d", GL_DOUBLE_MAT2x4, "Mat3x2d", GL_DOUBLE_MAT3x2, "Mat3x4d", GL_DOUBLE_MAT3x4, "Mat4x3d", GL_DOUBLE_MAT4x3, "Mat4x2d", GL_DOUBLE_MAT4x2,

			"Sampler1D", GL_SAMPLER_1D, "Sampler2D", GL_SAMPLER_2D, "Sampler3D", GL_SAMPLER_3D, "SamplerCube", GL_SAMPLER_CUBE, "Sampler1DShadow", GL_SAMPLER_1D_SHADOW, "Sampler2DShadow", GL_SAMPLER_2D_SHADOW,
			"Sampler1D[]", GL_SAMPLER_1D_ARRAY, "Sampler2D[]", GL_SAMPLER_2D_ARRAY, "Sampler1DShadow[]", GL_SAMPLER_1D_ARRAY_SHADOW, "Sampler2DShadow[]", GL_SAMPLER_2D_ARRAY_SHADOW,
			"Sampler2DMS", GL_SAMPLER_2D, "Sampler2DMS[]", GL_SAMPLER_2D_MULTISAMPLE, "SamplerCubeShadow", GL_SAMPLER_CUBE_SHADOW, "SamplerBuffer", GL_SAMPLER_BUFFER, "Sampler2DRect", GL_SAMPLER_2D_RECT, "Sampler2DRectShadow", GL_SAMPLER_2D_RECT_SHADOW,

			"Sampler1Di", GL_INT_SAMPLER_1D, "Sampler2Di", GL_INT_SAMPLER_2D, "Sampler3Di", GL_INT_SAMPLER_3D, "SamplerCubei", GL_INT_SAMPLER_CUBE, "Sampler1Di[]", GL_INT_SAMPLER_1D_ARRAY, "Sampler2Di[]", GL_INT_SAMPLER_2D_ARRAY,
			"Sampler2DMSi", GL_INT_SAMPLER_2D, "Sampler2DMSi[]", GL_INT_SAMPLER_2D_MULTISAMPLE, "SamplerBufferi", GL_INT_SAMPLER_BUFFER, "Sampler2DRecti", GL_INT_SAMPLER_2D_RECT,

			"Sampler1Du", GL_UNSIGNED_INT_SAMPLER_1D, "Sampler2Du", GL_UNSIGNED_INT_SAMPLER_2D, "Sampler3Du", GL_UNSIGNED_INT_SAMPLER_3D, "SamplerCubeu", GL_UNSIGNED_INT_SAMPLER_CUBE, "Sampler1Di[]", GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, "Sampler2Di[]", GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
			"Sampler2DMSu", GL_UNSIGNED_INT_SAMPLER_2D, "Sampler2DMSi[]", GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, "SamplerBufferu", GL_UNSIGNED_INT_SAMPLER_BUFFER, "Sampler2DRectu", GL_UNSIGNED_INT_SAMPLER_2D_RECT,

			"Image1D", GL_IMAGE_1D, "Image2D", GL_IMAGE_2D, "Image3D", GL_IMAGE_3D, "Image2DRect", GL_IMAGE_2D_RECT, "ImageCube", GL_IMAGE_CUBE, "ImageBuffer", GL_IMAGE_BUFFER,
			"Image1D[]", GL_IMAGE_1D_ARRAY, "Image2D[]", GL_IMAGE_2D_ARRAY, "Image2DMS", GL_IMAGE_2D_MULTISAMPLE, "Image2DMS[]", GL_IMAGE_2D_MULTISAMPLE_ARRAY,

			"Image1Di", GL_INT_IMAGE_1D, "Image2Di", GL_INT_IMAGE_2D, "Image3Di", GL_INT_IMAGE_3D, "Image2DRecti", GL_INT_IMAGE_2D_RECT, "ImageCubei", GL_INT_IMAGE_CUBE, "ImageBufferi", GL_INT_IMAGE_BUFFER,
			"Image1Di[]", GL_INT_IMAGE_1D_ARRAY, "Image2Di[]", GL_INT_IMAGE_2D_ARRAY, "Image2DMSi", GL_INT_IMAGE_2D_MULTISAMPLE, "Image2DMSi[]", GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

			"Image1Du", GL_UNSIGNED_INT_IMAGE_1D, "Image2Du", GL_UNSIGNED_INT_IMAGE_2D, "Image3Du", GL_UNSIGNED_INT_IMAGE_3D, "Image2DRectu", GL_UNSIGNED_INT_IMAGE_2D_RECT, "ImageCubeu", GL_UNSIGNED_INT_IMAGE_CUBE, "ImageBufferu", GL_UNSIGNED_INT_IMAGE_BUFFER,
			"Image1Du[]", GL_UNSIGNED_INT_IMAGE_1D_ARRAY, "Image2Du[]", GL_UNSIGNED_INT_IMAGE_2D_ARRAY, "Image2DMSu", GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE, "Image2DMSu[]", GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

			"aUint", GL_UNSIGNED_INT_ATOMIC_COUNTER
		);

		struct ShaderInputHelper {

			///Returns Undefined when it is not of default types
			static ShaderInputType getBase(ShaderInputType t) {
				u32 i = t.getIndex();
				if (i == 0) return ShaderInputType();
				--i;
				if (i < 4) return ShaderInputType::get(1);		//Float[x]
				if (i < 8) return ShaderInputType::get(5);		//Double[x]
				if (i < 12) return ShaderInputType::get(9);		//Int[x]
				if (i < 16) return ShaderInputType::get(13);	//Uint[x]
				if (i < 20) return ShaderInputType::get(17);	//Bool[x]
				if (i < 29) return ShaderInputType::get(1);		//Float[x][y]
				if (i < 38) return ShaderInputType::get(5);		//Double[x][y]
				return ShaderInputType();
			}

			///Returns 0 when it is not of default types
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

		struct ShaderInput {
			OString name;
			ShaderInputType type;
			u32 size;
		};
	}
}