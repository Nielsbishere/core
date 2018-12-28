#pragma once

#include "template/enum.h"
#include "graphics/objects/texture/texture.h"

namespace oi {

	namespace gc {

		class Graphics;

		class ShaderStage;
		class GPUBuffer;

		struct ShaderInput {

			TextureFormat type;
			String name;

			ShaderInput(TextureFormat type, String name) : type(type), name(name) {}
			ShaderInput() : ShaderInput(0, "") {}

		};

		struct ShaderOutput {

			u32 id;
			TextureFormat type;
			String name;

			ShaderOutput(TextureFormat type, String name, u32 id) : type(type), name(name), id(id) {}
			ShaderOutput() : ShaderOutput(TextureFormat::Undefined, "", 0U) {}

		};

		UEnum(ShaderRegisterType,
			Undefined = 0,
			UBO = 1, SSBO = 2,
			Texture2D = 3, Image = 4,
			Sampler = 5
		);

		enum class ShaderAccessType : u32 {

			COMPUTE = 0,

			VERTEX = 1,
			FRAGMENT = 2,
			GEOMETRY = 4,
			TESSELATION = 8,
			TESSELATION_EVALUATION = 16,

			MESH = 256,
			TASK = 512,

			RAY_GEN = 1024,
			ANY_HIT = 2048,
			CLOSEST_HIT = 4096,
			MISS = 8192,
			INTERSECTION = 16384,
			CALLABLE = 32768

		};

		struct ShaderRegister {

			ShaderRegisterType type;
			ShaderAccessType access;
			u32 size, id;
			TextureFormat format;
			String name;

			ShaderRegister(ShaderRegisterType type, ShaderAccessType access, String name, u32 size, u32 id, TextureFormat format) : type(type), access(access), name(name), size(size), id(id), format(format) {}
			ShaderRegister() : ShaderRegister(0, ShaderAccessType::COMPUTE, "", 0, 0, TextureFormat::Undefined) { }

			bool operator==(const ShaderRegister &other) const { return memcmp(this, &other, offsetof(ShaderRegister, name)) == 0 && name == other.name; }

		};

	}

}