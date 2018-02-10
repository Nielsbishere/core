#include "Graphics/Material/OpenGLShader.h"
#include "Graphics/OpenGLGraphics.h"
#include "Graphics/GPU/OpenGLBufferGPU.h"
#include "API/OpenGL.h"

using namespace oi::gc;
using namespace oi;

OpenGLShader::OpenGLShader(ShaderInfo info) : Shader(info), shaderId(0) {}

bool OpenGLShader::init() {

	if (info.getPath().getExtension() != "")
		return false;

	bool success = true;

	ShaderStageData *stages[(u32)ShaderStage::LENGTH];

	for (u32 i = 0; i < (u32)ShaderStage::LENGTH; ++i)
		stages[i] = nullptr;

	if (info.getType() == ShaderType::NORMAL) {

		if (info.hasVertexShader())
			success = (stages[0] = compile(info, ShaderStage::VERTEX)) != 0;

		if (info.hasFragmentShader() && success)
			success = (stages[1] = compile(info, ShaderStage::FRAGMENT)) != 0;

		if (info.hasGeometryShader() && success)
			success = (stages[2] = compile(info, ShaderStage::GEOMETRY)) != 0;

	}
	else
		success = (stages[3] = compile(info, ShaderStage::COMPUTE)) != 0;

	if (success)
		success = link(stages, (u32)ShaderStage::LENGTH);

	for (u32 i = 0; i < (u32)ShaderStage::LENGTH; ++i)
		cleanup(stages[i]);

	if (success)
		success = genReflectionData();

	return success;
}

void OpenGLShader::bind() {
	OpenGL::glUseProgram(shaderId);
}

void OpenGLShader::unbind() {
	OpenGL::glUseProgram(0);
}

OpenGLShader::~OpenGLShader() {
	destroy();
}

void OpenGLShader::destroy() {
	if (shaderId != 0) {
		OpenGL::glDeleteProgram(shaderId);
		attributes.clear();
		for (auto &elem : ssbos) {
			elem.second.buffer->destroy();
			elem.second.structured = StructuredBuffer(Buffer::construct(nullptr, 0));
		}
		ssbos.clear();
		shaderId = 0;
	}
}

void OpenGLShader::cleanup(ShaderStageData *shader) {
	if (shader != 0) {
		OpenGL::glDeleteShader(((OpenGLShaderStage*)shader)->id);
		delete shader;
		shader = nullptr;
	}
}

GLenum OpenGLShader::pickFromStage(ShaderStage which) {
	switch (which) {
	case ShaderStage::VERTEX:
		return GL_VERTEX_SHADER;
	case ShaderStage::FRAGMENT:
		return GL_FRAGMENT_SHADER;
	case ShaderStage::GEOMETRY:
		return GL_GEOMETRY_SHADER;
	case ShaderStage::COMPUTE:
		return GL_COMPUTE_SHADER;
	default:
		return 0;
	}
}

OString OpenGLShader::getExtension(ShaderStage which) {
	switch (which) {
	case ShaderStage::VERTEX:
		return "vs";
	case ShaderStage::FRAGMENT:
		return "fs";
	case ShaderStage::GEOMETRY:
		return "gs";
	default:
		return "cs";
	}
}

ShaderStageData *OpenGLShader::compile(ShaderInfo &si, ShaderStage which) {

	GLenum type = pickFromStage(which);

	if (type == 0) {
		Log::error("Couldn't compile shader; invalid OpenGLShaderStage");
		return 0;
	}

	OString file = OString::readFromFile(si.getPath() + "." + getExtension(which) + ".glsl");

	if (file == "") {
		Log::error(OString("Couldn't compile shader; invalid path (expected: ") + si.getPath() + "." + getExtension(which) + ".glsl" + ")");
		return 0;
	}

	GLuint shader = OpenGL::glCreateShader(type);

	const char *source = file.c_str();
	OpenGL::glShaderSource(shader, 1, &source, nullptr);
	OpenGL::glCompileShader(shader);

	GLint success;
	OpenGL::glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		logError(shader);
		OpenGL::glDeleteShader(shader);
		return 0;
	}

	return new OpenGLShaderStage(shader);
}

bool OpenGLShader::link(ShaderStageData **data, u32 count) {
	shaderId = OpenGL::glCreateProgram();

	for (u32 i = 0; i < count; ++i)
		if (data[i] != nullptr)
			OpenGL::glAttachShader(shaderId, ((OpenGLShaderStage*)data[i])->id);

	OpenGL::glLinkProgram(shaderId);

	GLint success;

	OpenGL::glGetShaderiv(shaderId, GL_LINK_STATUS, &success);
	if (!success) {
		logError(shaderId);
		OpenGL::glDeleteProgram((u64)shaderId);
		return false;
	}

	return true;
}

bool OpenGLShader::logError(GLuint handle) {

	GLint infoLen = 0;
	OpenGL::glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLen);

	if (infoLen > 1) {

		char *infoLog = new char[infoLen];
		OpenGL::glGetShaderInfoLog(handle, infoLen, NULL, infoLog);
		Log::error(OString("Error compiling shader; ") + infoLog);
		delete[] infoLog;

		return true;
	}

	return false;
}

bool OpenGLShader::genReflectionData() {

	GLint maxLength;
	OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

	GLint amaxLength;
	OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &amaxLength);

	if (amaxLength > maxLength)
		maxLength = amaxLength;

	char *name = new char[maxLength + 1];

	GLint uniformCount;
	OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_UNIFORMS, &uniformCount);

	GLint uniformBlockCount;
	OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_UNIFORM_BLOCKS, &uniformBlockCount);

	if (uniformCount != 0 || uniformBlockCount != 0)
		return Log::error("Couldn't compile shader. Uniforms and uniform blocks aren't supported by Osomi Core; please use SSBO's instead. This is due to performance reasons; buffers are way faster than uniforms and uniforms are limiting in so many areas. Cross API compatibility is also easier without uniforms (DirectX doesn't support them).");

	GLint attribCount;
	OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_ATTRIBUTES, &attribCount);

	attributes = std::vector<ShaderInput>((u32)attribCount);

	for (GLint i = 0; i < attribCount; i++) {
		GLint size;
		GLenum type;
		OpenGL::glGetActiveAttrib(shaderId, i, maxLength, NULL, &size, &type, name);

		auto ttype = OpenGLShaderInputType::find((u32)type);

		ShaderInput &r = attributes[i];
		r.name = name;
		r.size = (u32)size;
		r.type = ttype.getName();

		if (ShaderInputHelper::getBase(r.type).getIndex() == 0)
			return Log::error("Expected a regular data type (float/double/uint/int/bool/mat)");
	}

	delete[] name;

	GLint activeSSBOs;
	OpenGL::glGetProgramInterfaceiv(shaderId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &activeSSBOs);
	OpenGL::glGetProgramInterfaceiv(shaderId, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxLength);

	GLint maxVarLength;
	OpenGL::glGetProgramInterfaceiv(shaderId, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &maxVarLength);

	ssbos = std::unordered_map<OString, ShaderStorageBuffer>(activeSSBOs);

	name = new char[maxLength + 1];
	char *varName = new char[maxVarLength + 1];

	for (u32 i = 0; i < (u32) activeSSBOs; ++i) {

		GLint nameLength;

		///Obtain binding, active variable and size of buffer
		GLenum props[] = { GL_BUFFER_BINDING, GL_NUM_ACTIVE_VARIABLES , GL_BUFFER_DATA_SIZE };
		GLsizei proplen[] = { 1, 1, 1 };
		GLint propout[3] = {};
		OpenGL::glGetProgramResourceiv(shaderId, GL_SHADER_STORAGE_BLOCK, i, sizeof(props) / sizeof(GLenum), props, sizeof(props) / sizeof(GLenum), proplen, propout);
		OpenGL::glGetProgramResourceName(shaderId, GL_SHADER_STORAGE_BLOCK, i, maxLength, &nameLength, name);

		OString sName = OString(name);

		///Obtain variables of buffer
		GLenum props0[] = { GL_ACTIVE_VARIABLES };
		GLsizei proplen0[] = { propout[1] };
		GLint *propout0 = new GLsizei[propout[1]];
		OpenGL::glGetProgramResourceiv(shaderId, GL_SHADER_STORAGE_BLOCK, i, sizeof(props0) / sizeof(GLenum), props0, propout[1], proplen0, propout0);

		u32 buflen = propout[2];

		///Obtain per variable information
		//std::unordered_map<OString, ShaderStorageVariable> vars(propout[1]);

		for (u32 j = 0; j < (u32) propout[1]; ++j) {

			GLint varId = propout0[j], varNameLength;

			///Obtain variable offset, type and array size/stride (if needed)
			GLenum props1[] = { GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE };
			GLsizei proplen1[] = { 1, 1, 1, 1, 1, 1 };
			GLint propout1[6] = {};
			OpenGL::glGetProgramResourceiv(shaderId, GL_BUFFER_VARIABLE, varId, sizeof(props1) / sizeof(GLenum), props1, sizeof(props1) / sizeof(GLenum), proplen1, propout1);
			OpenGL::glGetProgramResourceName(shaderId, GL_BUFFER_VARIABLE, varId, maxVarLength, &varNameLength, varName);

			OString sVarName = OString(varName, varNameLength);

			OpenGLShaderInputType varType = OpenGLShaderInputType_s((u32)propout1[0]);
			Log::println(sVarName + ": " + varType.getName() + " (at offset " + propout1[1] + "; " + propout1[2] + "/" + propout1[3] + ": " + propout1[4] + "/" + propout1[5] + ")");

			ShaderInputType type = varType.getName();

			//init stride & arraySize

			//vars[sVarName] = ShaderStorageVariable(type, propout1[1], ShaderInputHelper::getSize(type), 0, 0);
		}

		///Allocate BufferGPU and store type data
		//ssbos[sName] = ShaderStorageBuffer(new OpenGLBufferGPU(BufferType::SSBO, buflen, propout[0]), vars);

		delete[] propout0;

	}

	delete[] name;
	delete[] varName;

	return true;
}