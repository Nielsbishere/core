#include "Graphics/GraphicsInterface.h"
#include <Input/InputHandler.h>
#include "Graphics/GPU/BufferGPU.h"
#include "Graphics/GPU/BufferLayout.h"
#include <Types/Matrix.h>
using namespace oi::gc;
using namespace oi;

GraphicInterface::GraphicInterface() : gl(Graphics::get()) { }

void GraphicInterface::init() {
	gl->init(getParent());
	initScene();

	getInput().load("Resources/Settings/Input.json");
}

void GraphicInterface::render() {
	gl->clear(RGBAf(1, 1, 0, 1));
	renderScene();
	getParent()->swapBuffers();
	//Vec3 mov = getInput().getAxis("Move");
}

BufferGPU *vertexBuffer, *indexBuffer;
BufferLayout *bufferLayout;

void GraphicInterface::initScene() {
	s = gl->compileShader(ShaderInfo("Resources/Shaders/test", ShaderType::NORMAL));

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

	vertexBuffer = gl->createBuffer(BufferType::VBO, Buffer((u8*)vdata, sizeof(vdata)));
	indexBuffer = gl->createBuffer(BufferType::IBO, Buffer((u8*)idata, sizeof(idata)));
	bufferLayout = gl->createLayout(vertexBuffer);

	bufferLayout->add(ShaderInputType::Float3);
	bufferLayout->add(ShaderInputType::Float2);

	vertexBuffer->init();
	indexBuffer->init();
	bufferLayout->init(indexBuffer);

	Matrix m = Matrix().makeScale(Vec4(1, 2, 3, 1));
	int debug = 0;
}

void GraphicInterface::renderScene() {
	s->bind();
	bufferLayout->bind();
	gl->renderElement(Primitive::TriangleFan, 4);
	bufferLayout->unbind();
	s->unbind();
}

GraphicInterface::~GraphicInterface() {
	delete s;
	delete gl;
}