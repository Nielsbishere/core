#pragma once

#include "graphics/gl/generic.h"
#include "graphics/objects/graphicsobject.h"
#include "graphics/objects/shader/shaderdata.h"

namespace oi {

	namespace gc {

		class Shader;
		class PipelineState;
		class RenderTarget;
		class Camera;
		class MeshBuffer;
		class Pipeline;
		class GPUBuffer;

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

			PipelineExt &getExtension();
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

			template<typename T>
			void setValue(String path, const T &value);

			template<typename T>
			void getValue(String path, T &value);

			void setRegister(String path, GraphicsResource *res);
			
			template<typename T>
			T *getRegister(String path);

			void instantiateBuffer(String path, u32 objects);
			void setBuffer(String path, u32 elements, GPUBuffer *buffer);
			void setData(String path, Buffer data);

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
			PipelineExt *ext;

		};

		template<typename T>
		void Pipeline::setValue(String path, const T &value) {
			info.shaderData->setValue(path, value);
		}

		template<typename T>
		void Pipeline::getValue(String path, T &value) {
			info.shaderData->getValue(path, value);
		}

		template<typename T>
		T *Pipeline::getRegister(String path) {
			return info.shaderData->get<T>(path);
		}

	}

}