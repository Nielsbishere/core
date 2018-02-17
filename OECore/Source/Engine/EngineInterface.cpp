#include "Engine/EngineInterface.h"
#include <Input/InputManager.h>
#include <Graphics/GPU/BufferGPU.h>
#include <Graphics/GPU/BufferLayout.h>
#include <Graphics/Material/Texture.h>
#include <Graphics/Material/Sampler.h>
#include <Graphics/Material/Shader.h>
#include <Graphics/Graphics.h>
#include <API/stbi/stbi_load.h>

using namespace oi::ec;
using namespace oi::gc;
using namespace oi;

BufferGPU *vertexBuffer, *indexBuffer;
BufferLayout *bufferLayout;

f64 startTime = 0;

void EngineInterface::initScene() {

	assets.init("Resources/Scene.json");

	shader = assets.find("test").getAssetHandle();
	texture = assets.find("Osomi").getAssetHandle();
	sampler = assets.find("nearest").getAssetHandle();

	struct Vertex {
		Vec3 pos;
		Vec2 uv;
	};

	Vertex vdata[] = {

		//Front
		{ { -1, -1, 1 },{ 0, 0 } },{ { 1, -1, 1 },{ 1, 0 } },
		{ { 1, 1, 1 },{ 1, 1 } },{ { -1, 1, 1 },{ 0, 1 } },

		//Back
		{ { -1, -1, -1 },{ 1, 0 } },{ { 1, -1, -1 },{ 0, 0 } },
		{ { 1, 1, -1 },{ 0, 1 } },{ { -1, 1, -1 },{ 1, 1 } },

		//Left
		{ { -1, 1, -1 },{ 0, 0 } },{ { -1, 1, 1 },{ 1, 0 } },
		{ { -1, -1, 1 },{ 1, 1 } },{ { -1, -1, -1 },{ 0, 1 } },

		//Right
		{ { 1, 1, -1 },{ 1, 0 } },{ { 1, 1, 1 },{ 0, 0 } },
		{ { 1, -1, 1 },{ 0, 1 } },{ { 1, -1, -1 },{ 1, 1 } },

		//Down
		{ { -1, -1, 1 },{ 0, 0 } },{ { 1, -1, 1 },{ 1, 0 } },
		{ { 1, -1, -1 },{ 1, 1 } },{ { -1, -1, -1 },{ 0, 1 } },

		//Up
		{ { -1, 1, 1 },{ 1, 0 } },{ { 1, 1, 1 },{ 0, 0 } },
		{ { 1, 1, -1 },{ 0, 1 } },{ { -1, 1, -1 },{ 1, 1 } }

	};

	u32 idata[] = {
		0, 1, 2, 3,		//Front
		7, 6, 5, 4,		//Back
		8, 9, 10, 11,	//Left
		15, 14, 13, 12,	//Right
		16, 17, 18, 19,	//Down
		23, 22, 21, 20	//Up
	};

	vertexBuffer = gl->create(BufferInfo(BufferType::VBO, Buffer((u8*)vdata, sizeof(vdata))));
	indexBuffer = gl->create(BufferInfo(BufferType::IBO, Buffer((u8*)idata, sizeof(idata))));
	bufferLayout = gl->create(vertexBuffer);

	bufferLayout->add(ShaderInputType::Float3);
	bufferLayout->add(ShaderInputType::Float2);

	vertexBuffer->init();
	indexBuffer->init();
	bufferLayout->init(indexBuffer);
}

void EngineInterface::renderScene() {
	assets[shader]->bind();

	assets[texture]->bind();
	assets[sampler]->bind();

	bufferLayout->bind();
	gl->renderElement(Primitive::TriangleFan, 4);
	bufferLayout->unbind();

	assets[texture]->unbind();
	assets[sampler]->unbind();

	assets[shader]->unbind();
}

void EngineInterface::update(f64 delta) {

	Shader *s = assets.get<Shader>(shader);
	s->get("textureBuffer.a").toFloat3() = { (f32) sin(startTime) * 0.5f + 0.5f, (f32) sin(startTime * 0.5f)* 0.5f + 0.5f, (f32) sin(startTime * 0.3f) * 0.5f + 0.5f };
	s->get("textureBuffer.c").toFloat3() = { 0.5f, 0.5f, 1.0f };

	startTime += delta;
}