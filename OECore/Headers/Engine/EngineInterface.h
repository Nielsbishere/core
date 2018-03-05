#pragma once

#include <Graphics/GraphicsInterface.h>

namespace oi {

	namespace ec {

		class EngineInterface : public gc::GraphicsInterface {

		public:

			EngineInterface(gc::GraphicsManager &gm);
			void update(f64 dt) override;
			void initScene() override;
			void renderScene() override;

		};

	}

}