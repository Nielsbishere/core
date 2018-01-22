#pragma once

#include <Window/WindowInterface.h>
#include <Window/Window.h>
#include "Graphics.h"
#include "Material/Shader.h"

namespace oi {
	namespace gc {
		class GraphicInterface : public wc::WindowInterface {

		public:

			GraphicInterface(): gl(Graphics::get()) { }

			void init() override {
				gl->init(getParent());
				initScene();
			}

			void render() override {
				gl->clear(RGBAf(1, 1, 0, 1));
				renderScene();
				getParent()->swapBuffers();
			}

			virtual void initScene() {
				s = gl->compileShader(ShaderInfo("Resources/Shaders/test", ShaderType::NORMAL));

				struct Vertex {
					Vec3 pos;
				};

			}

			virtual void renderScene() {
				s->bind();
				
				s->unbind();
			}

			~GraphicInterface() {
				delete s;
				delete gl;
			}

		protected:

			Graphics *&gl;

		private:

			Shader *s;

		};

	}
}