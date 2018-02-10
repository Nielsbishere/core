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

BufferGPU *vertexBuffer, *indexBuffer, *dataBuffer;
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

	StructuredBuffer sb = StructuredBuffer(Buffer::construct((u8*)vdata, sizeof(vdata)));
	/*sb.add("vdata", GDataType::oi_struct, 0, sizeof(Vertex), sizeof(vdata) / sizeof(Vertex));
	sb.add("vdata.pos", GDataType::oi_float3, 0, 0, 1, &sb.find("vdata"));
	sb.add("vdata.pos.x", GDataType::oi_float, 0, 0, 1, &sb.find("vdata.pos"));
	sb.add("vdata.pos.y", GDataType::oi_float, 4, 0, 1, &sb.find("vdata.pos"));
	sb.add("vdata.pos.z", GDataType::oi_float, 8, 0, 1, &sb.find("vdata.pos"));
	sb.add("vdata.uv", GDataType::oi_float2, 12, 0, 1, &sb.find("vdata"));
	sb.add("vdata.uv.x", GDataType::oi_float2, 0, 0, 1, &sb.find("vdata.uv"));
	sb.add("vdata.uv.y", GDataType::oi_float2, 4, 0, 1, &sb.find("vdata.uv"));*/

	//Instead
	sb.addAll("vdata[24].pos", GDataType::oi_float3, 0);
	sb.addAll("vdata.uv", GDataType::oi_float2, 12);

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
	dataBuffer = gl->createBuffer(BufferType::SSBO, Buffer((u8*)&ssbo, sizeof(ssbo)), 0);
	bufferLayout = gl->createLayout(vertexBuffer);

	bufferLayout->add(ShaderInputType::Float3);
	bufferLayout->add(ShaderInputType::Float2);

	s->init();
	vertexBuffer->init();
	indexBuffer->init();
	dataBuffer->init();
	bufferLayout->init(indexBuffer);
}

void GraphicsInterface::renderScene() {
	s->bind();
	bufferLayout->bind();
	dataBuffer->bind();
	gl->renderElement(Primitive::TriangleFan, 4);
	dataBuffer->unbind();
	bufferLayout->unbind();
	s->unbind();
}

GraphicsInterface::~GraphicsInterface() {
	delete s;
	delete gl;
}