#pragma once

#include <Window/WindowInterface.h>
#include <Window/Window.h>
#include "Graphics.h"
#include "Material/Shader.h"

namespace oi {
	namespace gc {
		class GraphicInterface : public wc::WindowInterface {

		public:

			GraphicInterface();

			void init() override;
			void render() override;

			virtual void initScene();
			virtual void renderScene();

			~GraphicInterface();

		protected:

			Graphics *&gl;

		private:

			Shader *s;

		};

	}
}