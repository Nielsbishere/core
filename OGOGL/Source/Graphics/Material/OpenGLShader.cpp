#include "Graphics/Material/OpenGLShader.h"
#include "API/OpenGL.h"

using namespace oi::gc;
using namespace oi;

OpenGLShader::OpenGLShader() : shaderId(0) {}

bool OpenGLShader::init(ShaderInfo info) {

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
	if (shaderId != 0) {
		OpenGL::glDeleteProgram(shaderId);
		shaderId = 0;
	}
}

bool OpenGLShader::isValid() { return shaderId != 0; }

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

	GLint attribCount;
	OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_ATTRIBUTES, &attribCount);

	attributes = std::vector<ShaderInput>((u32)attribCount);
	uniforms = std::vector<ShaderInput>((u32)uniformCount);

	for (GLint i = 0; i < attribCount; i++) {
		GLint size;
		GLenum type;
		OpenGL::glGetActiveAttrib(shaderId, i, maxLength, NULL, &size, &type, name);

		ShaderInput &r = attributes[i];
		r.name = OString(name).split("[")[0];
		r.size = (u32)size;
		r.type = ShaderInputType::findValue((u32)type);

		if (ShaderInputHelper::getBase(r.type).getIndex() == 0)
			return Log::error("Auto generated VAO expected a regular data type (float/double/uint/int/bool/mat)");
	}

	for (GLint i = 0; i < uniformCount; i++) {
		GLint size;
		GLenum type;
		OpenGL::glGetActiveUniform(shaderId, i, maxLength, NULL, &size, &type, name);

		ShaderInput &r = uniforms[i];
		r.name = OString(name).split("[")[0];
		r.size = (u32)size;
		r.type = ShaderInputType::findValue((u32)type);
	}

	delete[] name;
	return true;
}