#pragma once

#include "graphics/gl/generic.h"
#include <template/enum.h>
#include <types/buffer.h>
#include "graphics/graphicsobject.h"
#include "graphics/shaderenums.h"

namespace oi {

	namespace gc {
		
		class Graphics;

		DEnum(ShaderStageType, u32, Undefined = 0, Vertex_shader = 1, Fragment_shader = 2, Geometry_shader = 3, Compute_shader = 4);

		class ShaderStage;

		struct ShaderStageInfo {

			typedef ShaderStage ResourceType;

			CopyBuffer code;
			ShaderStageType type;

			std::vector<ShaderInput> input;
			std::vector<ShaderOutput> output;

			ShaderStageInfo(CopyBuffer code, ShaderStageType type, std::vector<ShaderInput> input, std::vector<ShaderOutput> output) : code(code), type(type), input(input), output(output) {}
			ShaderStageInfo() {}
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