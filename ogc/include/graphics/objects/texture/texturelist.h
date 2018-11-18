#pragma once
#include "graphics/objects/graphicsresource.h"

namespace oi {

	namespace gc {

		class Texture;
		class TextureList;

		struct TextureListInfo {

			typedef TextureList ResourceType;

			std::vector<Texture*> textures;

			TextureListInfo(u32 count) : textures(count) {
				memset(textures.data(), 0, sizeof(Texture*) * textures.size());
			}

			TextureListInfo() {}

		};

		typedef u32 TextureHandle;

		class TextureList : public GraphicsResource {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			Texture *get(TextureHandle i);

			TextureHandle alloc(Texture *tex);
			void dealloc(Texture *tex);

			u32 size();

			const TextureListInfo getInfo() { return info; }

		protected:

			~TextureList() {}
			TextureList(TextureListInfo info) : info(info) {}
			bool init() { return true; }

		private:

			TextureListInfo info;

		};

	}

}