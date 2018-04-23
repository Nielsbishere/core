#pragma once

#include "gl/generic.h"

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

		class Pipeline {

			friend class Graphics;

		public:

			~Pipeline();

			PipelineExt &getExtension();
			const PipelineInfo &getInfo();

		protected:

			Pipeline(PipelineInfo info);
			bool init(Graphics *g);

		private:

			PipelineInfo info;
			Graphics *g = nullptr;

			PipelineExt ext;

		};

	}

}