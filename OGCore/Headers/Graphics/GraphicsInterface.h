#pragma once

#include <Window/WindowInterface.h>

namespace oi {

	namespace gc {

		class Graphics;
		class GraphicsManager;

		class GraphicsInterface : public wc::WindowInterface {

		public:

			GraphicsInterface(GraphicsManager &gm);
			virtual ~GraphicsInterface() {}

			void init() override;
			void render() override;

			virtual void initScene() = 0;
			virtual void renderScene() = 0;

		protected:

			Graphics *&gl;

		};

	}

}