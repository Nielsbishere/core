#pragma once

#include <Types/OString.h>
#include "ShaderType.h"
#include "ShaderStage.h"

namespace oi {
	namespace gc {

		class ShaderInfo {

			friend class Shader;

		public:

			ShaderInfo(ShaderType _type): type(_type), hasVS(false), hasFS(false), hasGS(false) {
				if (type == ShaderType::Normal)
					hasVS = hasFS = true;
			}

			void setHasVertex(bool b) {
				hasVS = b;
			}

			void setHasFragment(bool b) {
				hasFS = b;
			}

			void setHasGeometry(bool b) {
				hasGS = b;
			}

			bool hasVertexShader() const { return hasVS; }
			bool hasFragmentShader() const { return hasFS; }
			bool hasGeometryShader() const { return hasGS; }

			ShaderType getType() const { return type; }

		private:

			ShaderType type;
			bool hasVS, hasFS, hasGS;
		};

	}
}