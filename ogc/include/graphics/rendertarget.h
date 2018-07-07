#pragma once

#include "graphics/texture.h"
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		class VersionedTexture;

		struct RenderTargetInfo {

			Vec2u res;
			u32 buffering, targets;
			TextureFormat depthFormat;
			std::vector<TextureFormat> formats;

			Texture *depth;
			std::vector<VersionedTexture*> textures;

			RenderTargetInfo(Vec2u res, TextureFormat depth, std::vector<TextureFormat> formats, u32 buffering = 0) : res(res), depthFormat(depth), formats(formats), buffering(buffering), targets((u32) formats.size()) {}

		};

		class RenderTarget : public GraphicsObject {

			friend class Graphics;

		public:

			u32 getVersions();
			u32 getTargets();

			Texture *getDepth();
			VersionedTexture *getTarget(u32 target);

			Vec2u getSize();
			bool isOwned();

			RenderTargetExt &getExtension();
			const RenderTargetInfo getInfo();

		protected:

			~RenderTarget();
			RenderTarget(RenderTargetInfo info);
			bool init(bool isOwned = true);

		private:

			bool owned = true;

			RenderTargetInfo info;
			RenderTargetExt ext;

		};

	}

}