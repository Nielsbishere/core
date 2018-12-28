#pragma once

#include "graphics/gl/generic.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class Shader;
		class ShaderData;
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

		struct PipelineInfo {

			PipelineType type;
			GraphicsPipelineInfo graphicsInfo;
			ComputePipelineInfo computeInfo;
			RaytracingPipelineInfo raytracingInfo;
			ShaderData *shaderData = nullptr;

		};

		class Pipeline : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			PipelineExt &getExtension() { return ext; }
			const GraphicsPipelineInfo getGraphicsInfo() const { return info.graphicsInfo; }
			const ComputePipelineInfo getComputeInfo() const { return info.computeInfo; }
			const RaytracingPipelineInfo getRaytracingInfo() const { return info.raytracingInfo; }

			PipelineType getPipelineType() const { return info.type; }
			ShaderData *getData() const { return info.shaderData; }

			Shader *getShader(u32 i = 0) const;
			PipelineState *getPipelineState() const { return info.graphicsInfo.pipelineState; }
			RenderTarget *getRenderTarget() const { return info.graphicsInfo.renderTarget; }
			MeshBuffer *getMeshBuffer() const;
			u32 getRecursionDepth() const { return info.raytracingInfo.maxRecursionDepth; }

			void update();

		protected:

			~Pipeline();
			Pipeline(GraphicsPipelineInfo info) : info{ PipelineType::Graphics, info } {}
			Pipeline(ComputePipelineInfo info) : info{ PipelineType::Compute, {}, info } {}
			Pipeline(RaytracingPipelineInfo info) : info{ PipelineType::Raytracing, {}, {}, info } {}
			bool init();

			bool initData();
			void destroyData();

		private:

			PipelineInfo info;
			PipelineExt ext;

		};

	}

}