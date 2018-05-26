#pragma once

#include "graphics/texture.h"
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		struct RenderTargetInfo {

			Vec2u res;
			u32 buffering, targets;
			TextureFormat depthFormat;
			std::vector<TextureFormat> formats;

			Texture *depth;
			std::vector<Texture*> textures;

			RenderTargetInfo(Vec2u res, TextureFormat depth, std::vector<TextureFormat> formats, u32 buffering) : res(res), depthFormat(depth), formats(formats), buffering(buffering), targets((u32) formats.size() + 1) {}

		};

		class RenderTarget : public GraphicsObject {

			friend class Graphics;

		public:

			u32 getVersions();	//Get which buffering method is used; aka how many versions there are
			u32 getTargets();	//Get the number of targets per version

			Texture *getDepthBuffer();
			Texture *getTarget(u32 target, u32 version);

			Vec2u getSize();

			RenderTargetExt &getExtension();
			const RenderTargetInfo getInfo();

		protected:

			~RenderTarget();
			RenderTarget(RenderTargetInfo info);
			bool init();

		private:

			RenderTargetInfo info;
			RenderTargetExt ext;

		};

	}

}