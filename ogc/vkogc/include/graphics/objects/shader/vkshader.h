#pragma once
#include "vkshaderstage.h"
#include <vector>

namespace oi {

	namespace gc {

		class Shader;

		struct ShaderExt {

			typedef Shader BaseType;

			Array<ShaderStageExt*> stage;

		};

	}

}