#pragma once

#include "TextureInfo.h"
#include "Graphics/GPU/GraphicsResource.h"
#include <Types/Buffer.h>
#include <API/stbi/stbi_load.h>

namespace oi {
	
	namespace gc {

		class Texture : public GraphicsResource {


		public:

			Texture(Graphics *&_gl, TextureInfo ti) : GraphicsResource(_gl), info(ti) {}
			virtual ~Texture() { info.getBuffer().deconstruct(); }

			virtual u64 getTextureHandle() = 0;		//Initialize and/or return handle

			bool initData(OString path) override {

				if (path == "" || path == "-") return true;

				int desiredComp = 0;

				if (info.getLayout().getValue() == TextureLayout::RGBA || info.getLayout().getValue() == TextureLayout::RGBAc)
					desiredComp = 4;

				else if (info.getLayout().getValue() == TextureLayout::RGB || info.getLayout().getValue() == TextureLayout::RGBc)
					desiredComp = 3;

				else if (info.getLayout().getValue() == TextureLayout::RG || info.getLayout().getValue() == TextureLayout::RGc)
					desiredComp = 2;

				else if (info.getLayout().getValue() == TextureLayout::R || info.getLayout().getValue() == TextureLayout::Rc)
					desiredComp = 1;
				
				if (desiredComp == 0)
					return Log::error(OString("Asset with path \"") + path + "\" is not supported. TextureLayout doesn't support loading from file");

				int w, h, comp;

				u8 *dat = (u8*) stbi_load(path.c_str(), &w, &h, &comp, desiredComp);

				if (dat == nullptr)
					return Log::error(OString("Asset with path \"") + path + "\" doesn't exist");

				info = TextureInfo((u32)w, (u32)h, info.getLayout(), Buffer::construct(dat, w * h * desiredComp));
				return true;
			}

			void destroyData() override {
				if(info.getLayout().getValue() == TextureLayout::RGBA || info.getLayout().getValue() == TextureLayout::RGBAc)
					info.getBuffer().deconstruct();
			}

			TextureInfo getInfo() { return info; }

		protected:

			TextureInfo info;

		};

	}

}