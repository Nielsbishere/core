#pragma once

#include "graphics/gl/generic.h"
#include <template/enum.h>
#include <types/buffer.h>
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {
		
		class Graphics;

		DEnum(ShaderStageType, u32, Undefined = 0, Vertex_shader = 1, Fragment_shader = 2, Geometry_shader = 3, Compute_shader = 4);

		struct ShaderStageInfo {

			Buffer code;
			ShaderStageType type;

			ShaderStageInfo(Buffer code, ShaderStageType type) : code(code), type(type) {}
			ShaderStageInfo() : code(), type() {}
		};

		class ShaderStage : public GraphicsObject {

			friend class Graphics;

		public:

			const ShaderStageInfo getInfo();
			ShaderStageExt &getExtension();

		protected:

			~ShaderStage();
			ShaderStage(ShaderStageInfo info);
			bool init();

		private:

			ShaderStageInfo info;
			ShaderStageExt ext;

		};

	}

}