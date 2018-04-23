#pragma once

#include "graphics/gl/generic.h"
#include <template/enum.h>
#include <types/buffer.h>

namespace oi {

	namespace gc {
		
		class Graphics;

		DEnum(ShaderStageType, u32, Undefined = 0, Vertex_shader = 1, Fragment_shader = 2, Geometry_shader = 3, Compute_shader = 4);

		struct ShaderStageInfo {

			Buffer code;
			ShaderStageType type;

			ShaderStageInfo(Buffer code, ShaderStageType type) : code(code), type(type) {}

		};

		class ShaderStage {

			friend class Graphics;

		public:

			~ShaderStage();
			ShaderStageExt &getExtension();

		protected:

			ShaderStage(ShaderStageInfo info);
			bool init(Graphics *g);

		private:

			ShaderStageInfo info;
			Graphics *g = nullptr;

			ShaderStageExt ext;

		};

	}

}