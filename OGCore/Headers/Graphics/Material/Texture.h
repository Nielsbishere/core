#pragma once

#include "TextureInfo.h"
#include "Graphics/GPU/GraphicsResource.h"
#include <Types/Buffer.h>

namespace oi {
	
	namespace gc {

		class Texture : public GraphicsResource {


		public:

			Texture(TextureInfo ti, Buffer _buf, u32 _binding) : info(ti), buf(_buf), binding(_binding) {}
			virtual ~Texture() {}

			virtual u64 getTextureHandle() = 0;		//Initialize and/or return handle

			TextureInfo getInfo() { return info; }

			void setBinding(u32 _binding) { binding = _binding; }
			u32 getBinding() { return binding; }

		protected:

			TextureInfo info;
			u32 binding;
			Buffer buf;

		};

	}

}