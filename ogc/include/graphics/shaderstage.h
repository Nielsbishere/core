#pragma once

#include "graphics/gl/generic.h"
#include <template/enum.h>
#include <types/buffer.h>

namespace oi {

	namespace gc {
		
		class Graphics;

		DEnum(ShaderStageType, const char*, Undefined = "", Vertex_shader = "vert", Fragment_shader = "frag", Geometry_shader = "geom", Compute_shader = "comp");

		class ShaderStage {

			friend class Graphics;

		public:

			~ShaderStage();

		protected:

			ShaderStage(String name, ShaderStageType type);		//e.g. simple.frag, simple.vert; so no API dependent extensions (.spv, .glsl, .hlsl, etc.)
			ShaderStage(String name);							//Detect ShaderStageType from name
			bool init(Graphics *g);

		private:

			ShaderStageType type;
			String path;
			Graphics *g = nullptr;

			ShaderStageExt ext;

		};

	}

}