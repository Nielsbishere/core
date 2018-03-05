#pragma once

#include <Types/Generic.h>
#include "OGCore/Properties.h"
#include <Types/OString.h>
#include <Types/Buffer.h>
#include <Types/Vector.h>
#include "GPU/BufferType.h"
#include "Primitive.h"
#include "ResourceManager.h"

namespace oi {

	namespace wc {
		class Window;
	}

	namespace gc {
		
		class GraphicsManager;
		class Shader;
		class ShaderInfo;
		class BufferGPU;
		class BufferInfo;
		class BufferLayout;
		class BufferLayoutInfo;
		class Texture;
		class TextureInfo;
		class Sampler;
		class SamplerInfo;

		class Graphics {

		public:

			Graphics(GraphicsManager *_parent) : parent(_parent) {}
			virtual ~Graphics() {}
			virtual bool init(wc::Window *w) = 0;

			virtual void clear() = 0;
			virtual void clear(Vec3 color) = 0;
			virtual void viewport(wc::Window *w) = 0;

			virtual Shader *create(OString name, ShaderInfo info) = 0;
			virtual BufferGPU *create(OString name, BufferInfo info) = 0;
			virtual Texture *create(OString name, TextureInfo info) = 0;
			virtual Sampler *create(OString name, SamplerInfo info) = 0;
			virtual BufferLayout *create(OString name, BufferLayoutInfo info) = 0;

			virtual void renderElement(Primitive p, u32 length, u32 startIndex = 0) = 0;

			ResourceManager &getResources() { return manager; }

		protected:

			ResourceManager manager;
			GraphicsManager *parent;

		};

	}
}