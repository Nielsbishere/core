#pragma once

#include <Window/WindowInterface.h>
#include <Window/Window.h>
#include "Graphics.h"
#include "Material/Shader.h"

namespace oi {
	namespace gc {
		class GraphicInterface : public wc::WindowInterface {

		public:

			void init() override {
				gl.init(getParent());
				initScene();
			}

			void render() override {
				gl.clear();
				renderScene();
				getParent()->swapBuffers();
			}

			virtual void initScene() {
				s.init(ShaderInfo("Resources/Shaders/test", ShaderType::NORMAL));
			}

			virtual void renderScene() {
				s.bind();

				s.unbind();
			}

			~GraphicInterface() {
				s.destroy();
			}

		private:

			Graphics gl;
			Shader s;

		};

	}
}