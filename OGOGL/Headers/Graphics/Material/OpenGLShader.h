#pragma once

#include <Graphics/Material/Shader.h>
#include "OpenGLShaderInput.h"

namespace oi {

	namespace gc {

		class BufferGPU;

		struct OpenGLShaderStage : public ShaderStageData {
			GLuint id;

			OpenGLShaderStage(GLuint _id): id(_id) {}
		};

		class OpenGLShader : public Shader {

		public:

			OpenGLShader(Graphics *&gl, ShaderInfo info);
			~OpenGLShader();
			bool init() override;
			void destroy() override;
			void bind() override;
			void unbind() override;

		protected:

			void cleanup(ShaderStageData *stage) override;

			OString getExtension(ShaderStage stage) override;

			ShaderStageData *compile(ShaderInfo &si, ShaderStage which) override;
			bool link(ShaderStageData **data, u32 count) override;
			bool logError(GLuint handle);

			bool genReflectionData() override;

			GLenum pickFromStage(ShaderStage which);

		private:

			GLuint shaderId;
			std::vector<ShaderInput> attributes;

		};

	}
}