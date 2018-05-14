#pragma once

#include <template/enum.h>
#include "texture.h"

namespace oi {

	namespace gc {

		class Graphics;

		class ShaderStage;
		class GBuffer;

		struct ShaderVBSection {

			u32 stride;
			bool perInstance;

			ShaderVBSection(u32 stride, bool perInstance = false) : stride(stride), perInstance(perInstance) {}
			ShaderVBSection() : ShaderVBSection(0) {}

		};

		struct ShaderVBVar {

			u32 buffer, offset;
			TextureFormat type;
			String name;

			ShaderVBVar(u32 buffer, u32 offset, TextureFormat type, String name) : buffer(buffer), offset(offset), type(type), name(name) {}
			ShaderVBVar() : ShaderVBVar(0, 0, 0, "") {}

			bool operator<(const ShaderVBVar &other) const { return buffer < other.buffer || (buffer == other.buffer && offset < other.offset); }

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
			UBO = 1, UBO_write = 2, SSBO = 3, SSBO_write = 4,
			Texture2D = 5, Image = 6,
			Sampler = 7
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

			ShaderRegister(ShaderRegisterType type, ShaderRegisterAccess access, String name) : type(type), access(access), name(name) {}
			ShaderRegister() : ShaderRegister(0, 0, "") { }

		};

	}

}