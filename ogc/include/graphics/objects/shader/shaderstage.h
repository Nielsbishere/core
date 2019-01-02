#pragma once

#include "template/enum.h"
#include "types/buffer.h"
#include "graphics/generic.h"
#include "graphics/objects/graphicsobject.h"
#include "graphics/objects/shader/shaderenums.h"

namespace oi {

	namespace gc {

		class Graphics;

		UEnum(ShaderStageType,

			Compute_shader = 0,

			Vertex_shader = 1,
			Fragment_shader = 2,
			Geometry_shader = 3,
			Tesselation_shader = 4,
			Tesselation_evaluation_shader = 5,

			Mesh_shader = 9,
			Task_shader = 10,

			Ray_gen_shader = 11,
			Any_hit_shader = 12,
			Closest_hit_shader = 13,
			Miss_shader = 14,
			Intersection_shader = 15,
			Callable_shader = 16,

			Undefined = 17
		);

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
			friend class oi::BlockAllocator;

		public:

			const ShaderStageInfo getInfo();
			ShaderStageExt &getExtension();

		protected:

			~ShaderStage();
			ShaderStage(ShaderStageInfo info);
			bool init();

		private:

			ShaderStageInfo info;
			ShaderStageExt *ext;

		};

	}
}