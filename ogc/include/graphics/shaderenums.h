#pragma once

#include <template/enum.h>
#include "texture.h"

namespace oi {

	namespace gc {

		class Graphics;

		class ShaderStage;
		class GBuffer;

		struct ShaderVBVar {

			TextureFormat type;
			String name;

			ShaderVBVar(TextureFormat type, String name) : type(type), name(name) {}
			ShaderVBVar() : ShaderVBVar(0, "") {}

		};

		struct ShaderOutput {

			u32 id;
			TextureFormat type;
			String name;

			ShaderOutput(TextureFormat type, String name, u32 id) : type(type), name(name), id(id) {}
			ShaderOutput() : ShaderOutput(TextureFormat::Undefined, "", 0U) {}

		};

		DEnum(ShaderRegisterType, u32,
			Undefined = 0,
			UBO = 1, SSBO = 2,
			Texture2D = 3, Image = 4,
			Sampler = 5
		);

		DEnum(ShaderRegisterAccess, u32,

			Undefined = 0,
			Compute = 1,
			Vertex = 2,
			Geometry = 4,
			Fragment = 8,

			Vertex_fragment = 10,
			Vertex_geometry_fragment = 14,
			Vertex_geometry = 6,
			Geometry_fragment = 12

		);

		struct ShaderRegister {

			ShaderRegisterType type;
			ShaderRegisterAccess access;
			String name;
			u32 size;

			ShaderRegister(ShaderRegisterType type, ShaderRegisterAccess access, String name, u32 size) : type(type), access(access), name(name), size(size) {}
			ShaderRegister() : ShaderRegister(0, 0, "", 0) { }

		};

	}

}