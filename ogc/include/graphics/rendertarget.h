#pragma once

#include "graphics/texture.h"

namespace oi {

	namespace gc {

		struct RenderTargetInfo {

			Vec2u res;
			u32 buffering, targets;
			TextureFormat depthFormat;

			std::vector<TextureFormat> formats;

			RenderTargetInfo(Vec2u res, TextureFormat depth, std::vector<TextureFormat> formats, u32 buffering) : res(res), depthFormat(depth), formats(formats), buffering(buffering), targets((u32) formats.size() + 1) {}

		};

		class RenderTarget {

			friend class Graphics;

		public:

			u32 getVersions();	//Get which buffering method is used; aka how many versions there are
			u32 getTargets();	//Get the number of targets per version

			Texture *getDepthBuffer();
			Texture *getTarget(u32 target, u32 version);

			Vec2u getSize();
			RenderTargetExt &getExtension();

			~RenderTarget();

		protected:

			RenderTarget(RenderTargetInfo info, Texture *depth, std::vector<Texture*> textures);
			bool init(Graphics *g);

		private:

			RenderTargetInfo info;
			Texture *depth;
			std::vector<Texture*> textures;

			Graphics *g = nullptr;

			RenderTargetExt ext;

		};

	}

}