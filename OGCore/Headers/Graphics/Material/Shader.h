#pragma once

#include "ShaderData.h"

namespace oi {
	namespace gc {

		class Shader {

		public:

			bool init(ShaderInfo info) { return data.init(info); }
			void bind() { data.bind(); }
			void unbind() { data.unbind(); }
			void destroy() { data.destroy(); }
			bool isValid() { return data.isValid(); }
			const std::vector<ShaderInput> getInputs() { return data.getInputs(); }

		private:

			ShaderData<OI_GRAPHICS_TYPE> data;
		};

	}
}