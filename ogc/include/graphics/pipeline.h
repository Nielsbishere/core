#pragma once

#include "gl/generic.h"
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		class Shader;
		class PipelineState;
		class RenderTarget;

		struct PipelineInfo {

			Shader *shader;
			PipelineState *pipelineState;
			RenderTarget *renderTarget;

			//Graphics pipeline
			PipelineInfo(Shader *shader, PipelineState *pipelineState, RenderTarget *renderTarget) : shader(shader), pipelineState(pipelineState), renderTarget(renderTarget) {}

			//Compute pipeline
			PipelineInfo(Shader *shader) : PipelineInfo(shader, nullptr, nullptr) {}

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