#pragma once

#include "gl/generic.h"
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		class Shader;
		class PipelineState;
		class RenderTarget;
		class Camera;

		struct PipelineInfo {

			Shader *shader;
			PipelineState *pipelineState;
			RenderTarget *renderTarget;
			Camera *camera;

			//Graphics pipeline
			PipelineInfo(Shader *shader, PipelineState *pipelineState, RenderTarget *renderTarget, Camera *camera) : shader(shader), pipelineState(pipelineState), renderTarget(renderTarget), camera(camera) {}

			//Compute pipeline
			PipelineInfo(Shader *shader) : PipelineInfo(shader, nullptr, nullptr, nullptr) {}

		};

		class Pipeline : public GraphicsObject {

			friend class Graphics;

		public:

			PipelineExt &getExtension();
			const PipelineInfo getInfo();

		protected:

			~Pipeline();
			Pipeline(PipelineInfo info);
			bool init();

		private:

			PipelineInfo info;
			PipelineExt ext;

		};

	}

}