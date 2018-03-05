#pragma once

#include <Types/Generic.h>
#include "OGCore/Properties.h"
#include "ShaderStage.h"
#include "Graphics/Material/ShaderInfo.h"
#include "Graphics/Material/ShaderInput.h"
#include "Graphics/GPU/GraphicsResource.h"
#include "Graphics/GPU/BufferGPU.h"
#include <Types/StructuredBuffer.h>

namespace oi {

	class Buffer;

	namespace gc {

		class BufferGPU;

		struct ShaderStageData { 
			virtual ~ShaderStageData() {}
		};

		struct ShaderStorageBuffer {

			BufferGPU *buffer;
			StructuredBuffer structured;

			ShaderStorageBuffer() : buffer(nullptr), structured(Buffer::construct(nullptr, 0)) {}
			ShaderStorageBuffer(BufferGPU *_buffer, StructuredBuffer _structured) : buffer(_buffer), structured(_structured) {}
		};

		class Shader : public GraphicsResource {

		public:

			Shader(Graphics *&_gl, ShaderInfo _info) : GraphicsResource(_gl), info(_info) {}
			virtual ~Shader() {}

			bool initData(OString _path) override { path = _path; return true; }

			BufferVar get(OString path);
			std::vector<OString> getBufferNames();
			std::vector<ShaderStorageBuffer> getBuffers();

		protected:

			virtual void cleanup(ShaderStageData *stage) = 0;

			virtual OString getExtension(ShaderStage stage) = 0;

			virtual ShaderStageData *compile(ShaderInfo &si, ShaderStage which) = 0;
			virtual bool link(ShaderStageData **data, u32 count) = 0;

			virtual bool genReflectionData() = 0;

			ShaderInfo info;

			OString path;
			std::unordered_map<OString, ShaderStorageBuffer> ssbos;

		};
	}
}