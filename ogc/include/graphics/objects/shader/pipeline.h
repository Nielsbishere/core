#pragma once

#include "graphics/gl/generic.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class Shader;
		class PipelineState;
		class RenderTarget;
		class Camera;
		class MeshBuffer;
		class Pipeline;

		struct PipelineInfo {

			typedef Pipeline ResourceType;

			Shader *shader;
			PipelineState *pipelineState;
			RenderTarget *renderTarget;
			MeshBuffer *meshBuffer;

			//Graphics pipeline
			PipelineInfo(Shader *shader, PipelineState *pipelineState, RenderTarget *renderTarget, MeshBuffer *meshBuffer) : shader(shader), pipelineState(pipelineState), renderTarget(renderTarget), meshBuffer(meshBuffer) {}

			//Compute pipeline
			PipelineInfo(Shader *shader) : PipelineInfo(shader, nullptr, nullptr, nullptr) {}

		};

		class Pipeline : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

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