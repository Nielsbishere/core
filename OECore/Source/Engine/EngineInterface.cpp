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

Shader *s;
BufferGPU *vertexBuffer, *indexBuffer;
BufferLayout *bufferLayout;
Texture *texture;
Sampler *sampler;

f64 startTime = 0;

void EngineInterface::initScene() {

	getInput().load("Resources/Settings/Input.json");

	s = gl->createShader(ShaderInfo("Resources/Shaders/test", ShaderType::NORMAL));

	OString name = "Resources/Images/Osomi.png";
	int w, h, comp;

	u8 *texdat = stbi_load(name.c_str(), &w, &h, &comp, 4);

	sampler = gl->createSampler(SamplerInfo(SamplerWrapping::ClampBorder, SamplerMin::Nearest, SamplerMag::Nearest));
	texture = gl->createTexture(TextureInfo((u32)w, (u32)h, TextureLayout::RGBA), Buffer::construct(texdat, (u32)(w * h * 4)));

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

	vertexBuffer = gl->createBuffer(BufferType::VBO, Buffer((u8*)vdata, sizeof(vdata)));
	indexBuffer = gl->createBuffer(BufferType::IBO, Buffer((u8*)idata, sizeof(idata)));
	bufferLayout = gl->createLayout(vertexBuffer);

	bufferLayout->add(ShaderInputType::Float3);
	bufferLayout->add(ShaderInputType::Float2);

	sampler->init();
	texture->init();
	s->init();
	vertexBuffer->init();
	indexBuffer->init();
	bufferLayout->init(indexBuffer);
}

void EngineInterface::renderScene() {
	s->bind();

	texture->bind();
	sampler->bind();

	bufferLayout->bind();
	gl->renderElement(Primitive::TriangleFan, 4);
	bufferLayout->unbind();

	texture->unbind();
	sampler->unbind();

	s->unbind();
}

void EngineInterface::update(f64 delta) {

	for (u32 i = 0; i < 4; ++i) {

		Buffer b = vertexBuffer->subbuffer(20 * i, 12);

		b.operator[]<Vec3>(0) *= 0.999f;
	}

	startTime += delta;
}