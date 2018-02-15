#pragma once
#include <Graphics/Material/Sampler.h>
#include "API/OpenGL.h"

namespace oi {

	namespace gc {

		class OpenGLSampler : public Sampler {

		public:

			OpenGLSampler(SamplerInfo ti);
			~OpenGLSampler();

			bool init() override;
			void destroy() override;
			void bind() override;
			void unbind() override;

		private:
			
			GLuint handle;
		};
	}

}