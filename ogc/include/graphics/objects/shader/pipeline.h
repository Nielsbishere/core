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
			Camera *camera;
			MeshBuffer *meshBuffer;

			//Graphics pipeline
			PipelineInfo(Shader *shader, PipelineState *pipelineState, RenderTarget *renderTarget, MeshBuffer *meshBuffer, Camera *camera) : shader(shader), pipelineState(pipelineState), renderTarget(renderTarget), camera(camera), meshBuffer(meshBuffer) {}

			//Compute pipeline
			PipelineInfo(Shader *shader) : PipelineInfo(shader, nullptr, nullptr, nullptr, nullptr) {}

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