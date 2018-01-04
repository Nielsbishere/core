#pragma once

#include <Types/OString.h>
#include "ShaderType.h"
#include "ShaderStage.h"

namespace oi {
	namespace gc {

		class ShaderInfo {

		public:

			ShaderInfo(OString _path, ShaderType _type) : path(_path), type(_type), hasVS(false), hasFS(false), hasGS(false) {
				if (type == ShaderType::NORMAL)
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

			OString getPath() const { return path; }
			ShaderType getType() const { return type; }

		private:

			OString path;
			ShaderType type;
			bool hasVS, hasFS, hasGS;
		};

	}
}