#pragma once

#include <Types/Generic.h>
#include "OGCore/Properties.h"
#include "ShaderStage.h"
#include "Graphics/Material/ShaderInfo.h"

namespace oi {
	namespace gc {

		struct ShaderStageData { 
			virtual ~ShaderStageData() {}
		};

		class Shader {

		public:

			virtual ~Shader() {}

			virtual bool init(ShaderInfo info) = 0;
			virtual void bind() = 0;
			virtual void unbind() = 0;
			virtual bool isValid() = 0;

		protected:

			virtual void cleanup(ShaderStageData *stage) = 0;

			virtual OString getExtension(ShaderStage stage) = 0;

			virtual ShaderStageData *compile(ShaderInfo &si, ShaderStage which) = 0;
			virtual bool link(ShaderStageData **data, u32 count) = 0;

			virtual bool genReflectionData() = 0;

		};
	}
}