#pragma once

#include <Types/Generic.h>
#include "OGCore/Properties.h"
#include "API/OpenGL.h"
#include "ShaderInfo.h"
#include "ShaderInput.h"

namespace oi {
	namespace gc {

		template<u32 gid>
		class ShaderData {};

		#ifdef CAN_HANDLE_OPENGL
		template<>
		class ShaderData<0> {

		protected:

			struct ShaderStageData {
				GLuint shader;
				ShaderStageData() : shader(0) {}
				ShaderStageData(GLuint d) : shader(d) {}
			};

		public:

			ShaderData() : shaderId(0) {}

			bool init(ShaderInfo info) {

				if (info.getPath().getExtension() != "")
					return false;

				bool success = true;

				ShaderStageData stages[(u32)ShaderStage::LAST];

				if (info.getType() == ShaderType::NORMAL) {

					if (info.hasVertexShader())
						success = (stages[0] = compile(info, ShaderStage::VERTEX)).shader != 0 && success;

					if (info.hasFragmentShader())
						success = (stages[1] = compile(info, ShaderStage::FRAGMENT)).shader != 0 && success;

					if (info.hasGeometryShader())
						success = (stages[2] = compile(info, ShaderStage::GEOMETRY)).shader != 0 && success;

				}
				else
					success = (stages[3] = compile(info, ShaderStage::COMPUTE)).shader != 0 && success;

				if (success)
					success = link(stages);

				for (u32 i = 0; i < (u32)ShaderStage::LAST; ++i)
					cleanup(stages[i]);

				if (success)
					success = getReflectionData();

				return success;
			}

			void bind() {
				OpenGL::glUseProgram(shaderId);
			}

			void unbind() {
				OpenGL::glUseProgram(0);
			}

			void destroy() {
				if (shaderId != 0) {
					OpenGL::glDeleteProgram(shaderId);
					shaderId = 0;
				}
			}

			bool isValid() { return shaderId != 0; }

			const std::vector<ShaderInput> getInputs() { return reflectionData; }

		protected:

			void cleanup(ShaderStageData &ssd) {
				if (ssd.shader != 0) {
					OpenGL::glDeleteShader(ssd.shader);
					ssd.shader = 0;
				}
			}

			GLenum pickFromStage(ShaderStage which) {
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

			OString getExtension(ShaderStage which) {
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

			ShaderStageData compile(ShaderInfo &si, ShaderStage which) {

				GLenum type = pickFromStage(which);

				if (type == 0) {
					Log::error("Couldn't compile shader; invalid ShaderStage");
					return ShaderStageData();
				}

				OString file = OString::readFromFile(si.getPath() + "." + getExtension(which) + ".glsl");

				if (file == "") {
					Log::error(OString("Couldn't compile shader; invalid path (expected: ") + si.getPath() + "." + getExtension(which) + ".glsl" + ")");
					return ShaderStageData();
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
					return ShaderStageData();
				}

				return ShaderStageData(shader);
			}

			bool link(ShaderStageData data[(u32)ShaderStage::LAST]) {
				shaderId = OpenGL::glCreateProgram();

				for (u32 i = 0; i < (u32)ShaderStage::LAST; ++i)
					if(data[i].shader != 0)
						OpenGL::glAttachShader(shaderId, data[i].shader);

				OpenGL::glLinkProgram(shaderId);

				GLint success;

				OpenGL::glGetShaderiv(shaderId, GL_LINK_STATUS, &success);
				if (!success) {
					logError(shaderId);
					OpenGL::glDeleteProgram(shaderId);
					return false;
				}

				return true;
			}

			bool logError(GLuint handle) {

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

			bool getReflectionData() {

				GLint maxLength;
				OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
				char *name = new char[maxLength + 1];

				GLint uniformCount;
				OpenGL::glGetProgramiv(shaderId, GL_ACTIVE_UNIFORMS, &uniformCount);

				reflectionData = std::vector<ShaderInput>(uniformCount);

				for (GLint i = 0; i < uniformCount; i++) {
					GLint size;
					GLenum type;
					OpenGL::glGetActiveUniform(shaderId, i, maxLength, NULL, &size, &type, name);

					ShaderInput &r = reflectionData[i];
					r.name = OString(name).split("[")[0];
					r.size = (u32)size;
					r.type = ShaderInputType::findValue((u32)type);
				}

				delete[] name;
				return true;
			}

		private:

			GLuint shaderId;
			std::vector<ShaderInput> reflectionData;

		};
		#endif

		#ifdef CAN_HANDLE_DX11
		template<>
		class ShaderData<1> {


		};
		#endif
	}
}