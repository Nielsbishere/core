#pragma once

#include "types/bitset.h"
#include "graphics/gl/generic.h"
#include "graphics/objects/graphicsobject.h"
#include "graphics/objects/shader/shaderbuffer.h"
#include "graphics/objects/shader/shaderstage.h"

namespace oi {

	namespace gc {

		class GraphicsResource;
		class Shader;
		class ShaderStageType;

		struct ShaderInfo {

			typedef Shader ResourceType;

			String path;

			std::vector<ShaderStage*> stage;
			std::vector<ShaderStageInfo> stages;

			std::vector<ShaderInput> inputs;
			std::vector<ShaderOutput> outputs;

			std::vector<ShaderRegister> registers;

			std::unordered_map<String, ShaderBufferInfo> buffer;

			Vec3u computeThreads;

			ShaderInfo(String path) : path(path) {}
			ShaderInfo() : ShaderInfo("") {}

		};

		class Shader : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			ShaderExt &getExtension();
			const ShaderInfo &getInfo();

			Vec3u getComputeThreads();

			static bool isCompatible(ShaderStageType t0, ShaderStageType t1);

		protected:

			~Shader();
			Shader(ShaderInfo info);
			bool init();

			bool initData();
			void destroyData();

		private:

			ShaderInfo info;
			ShaderExt ext;

		};

	}

}