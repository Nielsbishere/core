#pragma once

#include <Window/WindowInterface.h>
#include <Window/Window.h>
#include "Graphics.h"
#include "Material/Shader.h"

namespace oi {
	namespace gc {
		class GraphicsInterface : public wc::WindowInterface {

		public:

			GraphicsInterface();

			void init() override;
			void render() override;
			void update(f64 dt) override;

			virtual void initScene();
			virtual void renderScene();

			~GraphicsInterface();

		protected:

			Graphics *&gl;

		private:

			Shader *s;

		};

	}
}