#pragma once

#include "gl/generic.h"
#include "texture.h"
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		class Graphics;

		class ShaderStage;

		struct ShaderVBSection {
			
			u32 stride;
			bool perInstance;

			ShaderVBSection(u32 stride, bool perInstance = false) : stride(stride), perInstance(perInstance) {}
			ShaderVBSection() : ShaderVBSection(0) {}

		};

		struct ShaderVBVar {

			u32 buffer, offset;
			TextureFormat type;

			ShaderVBVar(u32 buffer, u32 offset, TextureFormat type) : buffer(buffer), offset(offset), type(type) {}
			ShaderVBVar() : ShaderVBVar(0, 0, 0) {}

		};

		struct ShaderInfo {

			String path;

			std::vector<ShaderStage*> stage;
			std::vector<ShaderVBSection> section;
			std::vector<ShaderVBVar> var;

			ShaderInfo(String path): path(path) {}

		};

		class Shader : public GraphicsObject {

			friend class Graphics;

		public:

			ShaderExt &getExtension();
			const ShaderInfo &getInfo();

			bool isCompute();

		protected:

			~Shader();
			Shader(ShaderInfo info);
			bool init();

		private:

			ShaderInfo info;
			ShaderExt ext;

		};

	}

}