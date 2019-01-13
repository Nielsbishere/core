#pragma once

#include "graphics/objects/graphicsobject.h"
#include "graphics/objects/texture/texture.h"

namespace oi {

	namespace gc {

		class VersionedTexture;
		class RenderTarget;

		struct RenderTargetInfo {

			typedef RenderTarget ResourceType;

			Vec2u res;
			u32 targets;
			TextureFormat depthFormat;
			std::vector<TextureFormat> formats;

			Texture *depth = nullptr;
			std::vector<VersionedTexture*> textures;

			bool isComputeTarget = false;

			RenderTargetInfo(Vec2u res, TextureFormat depth, std::vector<TextureFormat> formats) : res(res), depthFormat(depth), formats(formats), targets((u32) formats.size()) {}
			RenderTargetInfo(Vec2u res, std::vector<TextureFormat> formats) : res(res), depthFormat(TextureFormat::Undefined), formats(formats), targets((u32) formats.size()), isComputeTarget(true) {}

		};

		class RenderTarget : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			u32 getVersions();
			u32 getTargets();

			Texture *getDepth();
			VersionedTexture *getTarget(u32 target);

			bool resize(Vec2u size);

			Vec2u getSize();
			bool isOwned();

			bool isComputeTarget();

			RenderTargetExt &getExtension();
			const RenderTargetInfo &getInfo();

		protected:

			~RenderTarget();
			RenderTarget(RenderTargetInfo info);
			bool init(bool isOwned = true);

			bool initData();
			void destroyData();

		private:

			bool owned = true;

			RenderTargetInfo info;
			RenderTargetExt *ext;

		};

	}

}