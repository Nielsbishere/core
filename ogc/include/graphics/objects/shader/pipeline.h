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

		struct GraphicsPipelineInfo {

			typedef Pipeline ResourceType;

			Shader *shader;
			PipelineState *pipelineState;
			RenderTarget *renderTarget;
			MeshBuffer *meshBuffer;

			GraphicsPipelineInfo(Shader *shader, PipelineState *pipelineState, RenderTarget *renderTarget, MeshBuffer *meshBuffer) :
				shader(shader), pipelineState(pipelineState), renderTarget(renderTarget), meshBuffer(meshBuffer) {}

			GraphicsPipelineInfo() : GraphicsPipelineInfo(nullptr, nullptr, nullptr, nullptr) {}

		};

		struct ComputePipelineInfo {

			typedef Pipeline ResourceType;

			Shader *shader;

			ComputePipelineInfo(Shader *shader = nullptr) : shader(shader) {}

		};

		struct RaytracingPipelineInfo {

			typedef Pipeline ResourceType;

			std::vector<Shader*> shaders;
			MeshBuffer *meshBuffer;
			u32 maxRecursionDepth;

			RaytracingPipelineInfo(std::vector<Shader*> shaders, MeshBuffer *meshBuffer, u32 maxRecursionDepth) :
				shaders(shaders), meshBuffer(meshBuffer), maxRecursionDepth(maxRecursionDepth) {}

			RaytracingPipelineInfo() : RaytracingPipelineInfo({}, nullptr, 0) {}

		};

		UEnum(PipelineType, Graphics = 0, Compute = 1, Raytracing = 2);

		class Pipeline : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			PipelineExt &getExtension() { return ext; }
			const GraphicsPipelineInfo getGraphicsInfo() const { return graphicsInfo; }
			const ComputePipelineInfo getComputeInfo() const { return computeInfo; }
			const RaytracingPipelineInfo getRaytracingInfo() const { return raytracingInfo; }

			PipelineType getPipelineType() const { return type; }

			Shader *getShader(u32 i = 0);
			PipelineState *getPipelineState();
			RenderTarget *getRenderTarget();
			MeshBuffer *getMeshBuffer();
			u32 getRecursionDepth();

		protected:

			~Pipeline();
			Pipeline(GraphicsPipelineInfo info) : type(PipelineType::Graphics), graphicsInfo(info) {}
			Pipeline(ComputePipelineInfo info) : type(PipelineType::Compute), computeInfo(info) {}
			Pipeline(RaytracingPipelineInfo info) : type(PipelineType::Raytracing), raytracingInfo(info) {}
			bool init();

			bool initData();

		private:

			PipelineType type;
			GraphicsPipelineInfo graphicsInfo;
			ComputePipelineInfo computeInfo;
			RaytracingPipelineInfo raytracingInfo;
			PipelineExt ext;

		};

	}

}