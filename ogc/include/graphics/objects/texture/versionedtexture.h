#pragma once
#include "texture.h"

namespace oi {

	namespace gc {

		class VersionedTexture;

		struct VersionedTextureInfo {

			typedef VersionedTexture ResourceType;

			u32 versions = 0;
			std::vector<Texture*> version;

			VersionedTextureInfo(std::vector<Texture*> version) : versions((u32)version.size()), version(version) {}
			VersionedTextureInfo() {}

		};

		class VersionedTexture : public TextureObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			TextureFormat getFormat();
			TextureUsage getUsage();
			Vec2u getSize();
			u32 getVersions();
			bool isOwned();

			Texture *getVersion(u32 i);

			bool getPixels(Vec2u start, Vec2u length, CopyBuffer &output);
			bool write(String path, Vec2u start = Vec2u(), Vec2u length = Vec2u());

			const VersionedTextureInfo getInfo();

		protected:

			~VersionedTexture();
			VersionedTexture(VersionedTextureInfo info);
			bool init();

		private:

			VersionedTextureInfo info;

		};

	}

}