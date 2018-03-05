#include "Engine/EngineInterface.h"
#include <Input/InputManager.h>
#include <Graphics/GPU/BufferGPU.h>
#include <Graphics/GPU/BufferLayout.h>
#include <Graphics/Material/Texture.h>
#include <Graphics/Material/Sampler.h>
#include <Graphics/Material/Shader.h>
#include <Graphics/Graphics.h>
#include <Graphics/Mesh/oiRM.h>

using namespace oi::ec;
using namespace oi::gc;
using namespace oi;

BufferGPU *vertexBuffer, *indexBuffer;
BufferLayout *bufferLayout;
Shader *shader;
Texture *texture;
Sampler *sampler;

f64 startTime = 0;

EngineInterface::EngineInterface(gc::GraphicsManager &gm) : GraphicsInterface(gm) {}

void EngineInterface::initScene() {

	shader = gl->create("test", ShaderInfo(ShaderType::Normal));
	texture = gl->create("Osomi.png", TextureInfo(TextureType::Texture2D, TextureLayout::RGBAc));
	sampler = gl->create("nearest", SamplerInfo(SamplerWrapping::ClampBorder, SamplerMin::NearestMipLinear, SamplerMag::Nearest));

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
		//7, 6, 5, 4,		//Back
		//8, 9, 10, 11,	//Left
		//15, 14, 13, 12,	//Right
		//16, 17, 18, 19,	//Down
		//23, 22, 21, 20	//Up
	};

	vertexBuffer = gl->create("vertexBuffer", BufferInfo(BufferType::VBO, Buffer((u8*)vdata, sizeof(vdata))));
	indexBuffer = gl->create("indexBuffer", BufferInfo(BufferType::IBO, Buffer((u8*)idata, sizeof(idata))));

	std::vector<BufferLayoutElement> elements(2);

	elements[0].type = GDataType::oi_float3;
	elements[0].name = "pos";
	elements[0].buffer = "vertexBuffer";
	elements[0].index = 0;

	elements[1].type = GDataType::oi_float2;
	elements[1].name = "uv";
	elements[1].buffer = "vertexBuffer";
	elements[1].index = 1;

	bufferLayout = gl->create("bufferLayout", BufferLayoutInfo(elements, "indexBuffer"));

	gl->getResources().initCPU();
	gl->getResources().initGPU();
}

void EngineInterface::renderScene() {
	shader->bind();

	texture->bind();
	sampler->bind();

	bufferLayout->bind();
	gl->renderElement(Primitive::TriangleFan, 4);
	bufferLayout->unbind();

	texture->unbind();
	sampler->unbind();

	shader->unbind();
}

void EngineInterface::update(f64 delta) {

	shader->get("textureBuffer.a").toFloat3() = { (f32) sin(startTime) * 0.5f + 0.5f, (f32) sin(startTime * 0.5f)* 0.5f + 0.5f, (f32) sin(startTime * 0.3f) * 0.5f + 0.5f };
	shader->get("textureBuffer.c").toFloat3() = { 0.5f, 0.5f, 1.0f };

	startTime += delta;
}