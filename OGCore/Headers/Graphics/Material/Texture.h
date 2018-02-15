#pragma once

#include "TextureInfo.h"
#include "Graphics/GPU/GraphicsResource.h"
#include <Types/Buffer.h>

namespace oi {
	
	namespace gc {

		class Texture : public GraphicsResource {


		public:

			Texture(TextureInfo ti) : info(ti) {}
			virtual ~Texture() { info.getBuffer().deconstruct(); }

			virtual u64 getTextureHandle() = 0;		//Initialize and/or return handle

			TextureInfo getInfo() { return info; }

		protected:

			TextureInfo info;

		};

	}

}