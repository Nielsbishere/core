#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/shaderdata.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/model/meshbuffer.h"
using namespace oi::gc;
using namespace oi;

Shader *Pipeline::getShader(u32 i) const {

	switch (info.type.getValue()) {

	case PipelineType::Compute.value:
		return info.computeInfo.shader;

	case PipelineType::Graphics.value:
		return info.graphicsInfo.shader;

	default:
		return info.raytracingInfo.shaders[i];

	}

}

MeshBuffer *Pipeline::getMeshBuffer() const {

	switch (info.type.getValue()) {

	case PipelineType::Graphics.value:
		return info.graphicsInfo.meshBuffer;

	case PipelineType::Raytracing.value:
		return info.raytracingInfo.meshBuffer;

	default:
		return nullptr;

	}

}

void Pipeline::update() {
	info.shaderData->requestUpdate();
}

bool Pipeline::init() {

	g->use(info.graphicsInfo.shader);
	g->use(info.graphicsInfo.meshBuffer);
	g->use(info.graphicsInfo.pipelineState);
	g->use(info.graphicsInfo.renderTarget);
	g->use(info.computeInfo.shader);

	for(Shader *shader : info.raytracingInfo.shaders)
		g->use(shader);

	g->use(info.raytracingInfo.meshBuffer);

	if (info.raytracingInfo.shaders.size() == 0) {
		const ShaderInfo &shaderInfo = getShader()->getInfo();
		g->use(info.shaderData = g->create(getName() + " ShaderData", ShaderDataInfo(shaderInfo.registers, shaderInfo.buffer)));
	}

	return initData();
}

Pipeline::~Pipeline() {

	g->destroy(info.graphicsInfo.shader);
	g->destroy(info.graphicsInfo.meshBuffer);
	g->destroy(info.graphicsInfo.pipelineState);
	g->destroy(info.graphicsInfo.renderTarget);
	g->destroy(info.computeInfo.shader);

	for (Shader *shader : info.raytracingInfo.shaders)
		g->destroy(shader);

	g->destroy(info.raytracingInfo.meshBuffer);
	g->destroy(info.shaderData);

	destroyData();

}