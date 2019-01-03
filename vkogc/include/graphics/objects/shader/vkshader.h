#pragma once
#include "vkshaderstage.h"
#include <vector>

namespace oi {

	namespace gc {

		class Shader;

		struct ShaderExt {

			typedef Shader BaseType;

			std::vector<ShaderStageExt*> stage;

		};

	}

}