#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/model/meshbuffer.h"
using namespace oi::gc;
using namespace oi;

Shader *Pipeline::getShader(u32 i) {

	switch (type.getValue()) {

	case PipelineType::Compute.value:
		return computeInfo.shader;

	case PipelineType::Graphics.value:
		return graphicsInfo.shader;

	default:
		return raytracingInfo.shaders[i];

	}

}

MeshBuffer *Pipeline::getMeshBuffer() {

	switch (type.getValue()) {

	case PipelineType::Graphics.value:
		return graphicsInfo.meshBuffer;

	case PipelineType::Raytracing.value:
		return raytracingInfo.meshBuffer;

	default:
		return nullptr;

	}

}

PipelineState *Pipeline::getPipelineState() { return graphicsInfo.pipelineState; }
RenderTarget *Pipeline::getRenderTarget() { return graphicsInfo.renderTarget; }

u32 Pipeline::getRecursionDepth() { return raytracingInfo.maxRecursionDepth; }

bool Pipeline::init() {
	g->use(graphicsInfo.shader);
	g->use(graphicsInfo.meshBuffer);
	g->use(graphicsInfo.pipelineState);
	g->use(graphicsInfo.renderTarget);
	g->use(computeInfo.shader);

	for(Shader *shader :raytracingInfo.shaders)
		g->use(shader);

	g->use(raytracingInfo.meshBuffer);
	return initData();
}