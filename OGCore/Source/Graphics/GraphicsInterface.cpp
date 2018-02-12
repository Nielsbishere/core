#include "Graphics/GraphicsInterface.h"
#include <Input/InputHandler.h>
#include "Graphics/GPU/BufferGPU.h"
#include "Graphics/GPU/BufferLayout.h"
#include <Types/StructuredBuffer.h>
using namespace oi::gc;
using namespace oi;

GraphicsInterface::GraphicsInterface() : gl(Graphics::get()) { }

void GraphicsInterface::init() {
	gl->init(getParent());
	initScene();

	getInput().load("Resources/Settings/Input.json");
}

BufferGPU *vertexBuffer, *indexBuffer;
BufferLayout *bufferLayout;

f64 startTime = 0;

void GraphicsInterface::update(f64 delta) {

	for (u32 i = 0; i < 4; ++i) {

		Buffer b = vertexBuffer->subbuffer(20 * i, 12);

		b.operator[]<Vec3>(0) *= 0.999f;
	}

	startTime += delta;
}

void GraphicsInterface::render() {
	gl->viewport(getParent());
	gl->clear(Vec3(1, 0, 1));
	renderScene();
	getParent()->swapBuffers();
	//Vec3 mov = getInput().getAxis("Move");
}

void GraphicsInterface::initScene() {

	s = gl->createShader(ShaderInfo("Resources/Shaders/test", ShaderType::NORMAL));

	struct Vertex {
		Vec3 pos;
		Vec2 uv;
	};

	Vertex vdata[] = {
		
		//Front
		{ { -1, -1, 1 }, { 0, 0 } }, { { 1, -1, 1 }, { 1, 0 } }, 
		{ { 1, 1, 1 }, { 1, 1 } }, { { -1, 1, 1 }, { 0, 1 } },

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

	struct SSBO {
		Vec3 discardColor;
		u32 textures_c;
	};
	
	SSBO ssbo = { { 1, 0, 0 }, 0 };

	vertexBuffer = gl->createBuffer(BufferType::VBO, Buffer((u8*)vdata, sizeof(vdata)));
	indexBuffer = gl->createBuffer(BufferType::IBO, Buffer((u8*)idata, sizeof(idata)));
	bufferLayout = gl->createLayout(vertexBuffer);

	bufferLayout->add(ShaderInputType::Float3);
	bufferLayout->add(ShaderInputType::Float2);

	s->init();
	vertexBuffer->init();
	indexBuffer->init();
	bufferLayout->init(indexBuffer);

	s->get("textureBuffer.filterColor").toFloat3() = { 0.2f, 0.5f, 0.1f };
}

void GraphicsInterface::renderScene() {
	s->bind();
	bufferLayout->bind();
	gl->renderElement(Primitive::TriangleFan, 4);
	bufferLayout->unbind();
	s->unbind();
}

GraphicsInterface::~GraphicsInterface() {
	delete s;
	delete gl;
}