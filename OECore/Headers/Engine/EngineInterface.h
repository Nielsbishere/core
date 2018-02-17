#pragma once

#include <Graphics/GraphicsInterface.h>
#include "AssetManager.h"

namespace oi {

	namespace ec {

		class EngineInterface : public gc::GraphicsInterface {

		public:

			void update(f64 dt) override;
			void initScene() override;
			void renderScene() override;

		private:

			AssetManager assets;

			u32 texture, shader, sampler;

		};

	}

}